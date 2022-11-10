//===-- TargetedExecutionManager.cpp --------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TargetedExecutionManager.h"

#include "ExecutionState.h"
#include "klee/Core/TerminationTypes.h"
#include "klee/Module/KInstruction.h"

using namespace llvm;
using namespace klee;

class LocatedEventManager {
  using FilenameCache = std::unordered_map<std::string, bool>;
  std::unordered_map<std::string, FilenameCache *> filenameCacheMap;
  FilenameCache *filenameCache = nullptr;

public:
  LocatedEventManager() {}

  void prefetchFindFilename(const std::string &filename) {
    auto it = filenameCacheMap.find(filename);
    if (it != filenameCacheMap.end())
      filenameCache = it->second;
    else
      filenameCache = nullptr;
  }

  bool isInside(Location &loc, const klee::FunctionInfo &fi) {
    bool isInside = false;
    if (filenameCache == nullptr) {
      isInside = loc.isInside(fi);
      filenameCache = new FilenameCache();
      filenameCacheMap.insert(std::make_pair(fi.file, filenameCache));
      filenameCache->insert(std::make_pair(loc.filename, isInside));
    } else {
      auto it = filenameCache->find(loc.filename);
      if (it == filenameCache->end()) {
        isInside = loc.isInside(fi);
        filenameCache->insert(std::make_pair(loc.filename, isInside));
      } else {
        isInside = it->second;
      }
    }
    return isInside;
  }
};

struct TargetedExecutionManager::TargetPreparator {
  TargetedExecutionManager *tem;
  KModule *kmodule;
  std::vector<LocatedEvent *> filenameLineLocations;
  std::vector<LocatedEvent *> functionOffsetLocations;

TargetPreparator(TargetedExecutionManager *tem, KModule *kmodule, PathForest *pathForest)
  : tem(tem), kmodule(kmodule) {
  collectLocations(pathForest);
}

void collectLocations(PathForest *pathForest) {
  std::unordered_set<LocatedEvent *> result;
  std::vector<PathForest *> q{pathForest};
  while (!q.empty()) {
    auto forest = q.back();
    q.pop_back();
    for (const auto &p : forest->layer) {
      result.insert(p.first);
      if (p.second != nullptr)
        q.push_back(p.second);
    }
  }
  for (auto loc : result) {
    if (loc->hasFunctionWithOffset())
      functionOffsetLocations.push_back(loc);
    else
      filenameLineLocations.push_back(loc);
  }
}

void prepareFilenameLineLocations(std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2targets) {
  auto infos = kmodule->infos.get();
  LocatedEventManager lem;
  for (const auto &kfunc : kmodule->functions) {
    const auto &fi = infos->getFunctionInfo(*kfunc->function);
    lem.prefetchFindFilename(fi.file);
    auto targets = new std::set<ref<Target> >();
    for (int i = filenameLineLocations.size() - 1; i >= 0; i--) {
      auto le = filenameLineLocations[i];
      auto &loc = le->location;
      auto error = le->error;
      if (!lem.isInside(loc, fi))
        continue;
      for (const auto &kblock : kfunc->blocks) {
        auto b = kblock.get();
        if (!loc.isInside(b))
          continue;

        auto it = tem->block2targets.find(b);
        std::unordered_map<ReachWithError, ref<Target> > *targetMap;
        ref<Target> target = nullptr;
        if (it == tem->block2targets.end()) {
          targetMap = new std::unordered_map<ReachWithError, ref<Target> >();
        } else {
          targetMap = it->second;
          auto itt = targetMap->find(error);
          if (itt != targetMap->end())
            target = itt->second;
        }
        if (target.isNull()) {
          target = Target::create(b, error);
          targetMap->insert(std::make_pair(error, target));
          tem->block2targets.insert(it, std::make_pair(b, targetMap));
          tem->target2location.insert(std::make_pair(target, le));
        }
        targets->insert(target);
      }
      if (targets->empty())
        continue;
      filenameLineLocations[i] = filenameLineLocations.back();
      filenameLineLocations.pop_back();
      loc2targets.insert(std::make_pair(le, targets));
      targets = new std::set<ref<Target> >();
    }
    delete targets;
  }
}

KBlock *resolveBlock(Location &loc) {
  auto instr = loc.initInstruction(kmodule);
  return instr->parent;
}

void prepareFunctionOffsetLocations(std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2targets) {
  std::map<Location, KBlock *> loc2block;
  for (auto loc : functionOffsetLocations) {
    KBlock *block = nullptr;
    auto it = loc2block.find(loc->location);
    if (it == loc2block.end()) {
      block = resolveBlock(loc->location);
      loc2block.insert(it, std::make_pair(loc->location, block));
    } else {
      block = it->second;
    }
    auto error2targetIt = tem->block2targets.find(block);
    std::unordered_map<klee::ReachWithError, klee::ref<klee::Target>> *error2target = nullptr;
    if (error2targetIt == tem->block2targets.end()) {
      error2target = new std::unordered_map<klee::ReachWithError, klee::ref<klee::Target>>();
      tem->block2targets.insert(error2targetIt, std::make_pair(block, error2target));
    } else {
      error2target = error2targetIt->second;
    }
    ref<Target> target = Target::create(block, loc->error);
    error2target->insert(std::make_pair(loc->error, target));
    tem->target2location.insert(std::make_pair(target, loc));
  }
  for (auto loc : functionOffsetLocations) {
    auto targets = new std::set<ref<Target> >();
    auto targetMap = tem->block2targets.at(loc2block.at(loc->location));
    for (auto &p : *targetMap)
      targets->insert(p.second);
    loc2targets.insert(std::make_pair(loc, targets));
  }
}

}; // struct TargetPreparator


std::vector<std::pair<KFunction *, ref<TargetForest> > >
TargetedExecutionManager::prepareTargets(KModule *kmodule, PathForest *pathForest) {
  pathForest->normalize();

  TargetPreparator all_locs(this, kmodule, pathForest);
  std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> loc2targets;
  all_locs.prepareFilenameLineLocations(loc2targets);
  all_locs.prepareFunctionOffsetLocations(loc2targets);

  std::vector<std::pair<KFunction *, ref<TargetForest> > > whitelists;
  for (const auto &funcAndPaths : pathForest->layer) {
    ref<TargetForest> whitelist = new TargetForest(funcAndPaths.second, loc2targets);
    auto kf = (*loc2targets[funcAndPaths.first]->begin())->getBlock()->parent;
    whitelists.emplace_back(kf, whitelist);
  }

  for (auto p : loc2targets)
    delete p.second;

  return whitelists;
}

void TargetedExecutionManager::stepTo(ExecutionState &state, KBlock *dst) {
  auto it = block2targets.find(dst);
  if (it == block2targets.end())
    return;
  state.targetsOfCurrentKBlock = it->second;
}

void TargetedExecutionManager::reportFalsePositives(bool noMoreStates) {
  std::unordered_set<LocatedEvent *> visited;
  for (const auto &blockAndLoc : target2location) {
    auto expectedLocation = blockAndLoc.second;
    if (visited.find(expectedLocation) != visited.end() || expectedLocation->isReported)
      continue;
    visited.insert(expectedLocation);

    if (!blockAndLoc.first->shouldFailOnThisTarget())
      continue;

    unsigned char confidenceRate = 0;
    if (noMoreStates) {
      confidenceRate = 100;
    } else {
      confidenceRate = 0; //TODO: [Aleksandr Misonizhnik], calculate it via KLEE coverage or something
    }
    assert(0 <= confidenceRate && confidenceRate <= 100);
    // klee_warning("%u%% False Positive at: %s", confidenceRate, expectedLocation->toString().c_str());
    klee_warning("False Positive at trace %u", expectedLocation->id);
  }
}

void TargetedExecutionManager::reportFalseNegative(ExecutionState &state) {
  auto info = state.prevPC->info;
  std::ostringstream out;
  out << "False Negative at: " << info->file << ':' << info->line << ':' << info->column;
  // out << "100% False Negative at: " << info->file << ':' << info->line << ':' << info->column;
  klee_warning("%s", out.str().c_str());
}

bool TargetedExecutionManager::reportTruePositive(ExecutionState &state, ReachWithError error) {
  if (!state.targetsOfCurrentKBlock)
    return false;

  auto it = state.targetsOfCurrentKBlock->find(error);
  if (it == state.targetsOfCurrentKBlock->end())
    return false;

  auto target = it->second;
  if (target->getBlock() != state.prevPC->parent) {
    return false;
  }
  auto expectedLocation = target2location[target.get()];
  if (expectedLocation->isReported)
    return true;
  if (!expectedLocation->location.isTheSameAsIn(state.prevPC))
    return false;

  state.error = error;
  klee_warning("True Positive at trace %u", expectedLocation->id);
  // klee_warning("100%% True Positive at: %s", expectedLocation->toString().c_str());
  expectedLocation->isReported = true;
  return true;
}

TargetedExecutionManager::~TargetedExecutionManager() {
  for (auto p : block2targets)
    delete p.second;
}
