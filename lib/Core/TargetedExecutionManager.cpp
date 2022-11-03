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


std::vector<LocatedEvent *> *collectLocations(PathForest *pathForest) {
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
  auto out = new std::vector<LocatedEvent *>();
  for (auto loc : result)
    out->push_back(loc);
  return out;
}


std::vector<std::pair<KFunction *, ref<TargetForest> > >
TargetedExecutionManager::prepareTargets(const KModule *kmodule, PathForest *pathForest) {
  pathForest->normalize();
  auto all_locs = collectLocations(pathForest);

  auto infos = kmodule->infos.get();
  std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> loc2targets;
  std::unordered_map<std::string, std::unordered_map<std::string, bool> > filenameCache;
  for (const auto &kfunc : kmodule->functions) {
    const auto &fi = infos->getFunctionInfo(*kfunc->function);
    const auto &cache = filenameCache.find(fi.file);
    auto targets = new std::set<ref<Target> >();
    for (int i = all_locs->size() - 1; i >= 0; i--) {
      auto le = (*all_locs)[i];
      auto &loc = le->location;
      auto error = le->error;
      bool isInside = false;
      if (cache == filenameCache.end()) {
        isInside = loc.isInside(fi);
        std::unordered_map<std::string, bool> c;
        c.insert(std::make_pair(loc.filename, isInside));
        filenameCache.insert(std::make_pair(fi.file, c));
      } else {
        auto it = cache->second.find(loc.filename);
        if (it == cache->second.end()) {
          isInside = loc.isInside(fi);
          cache->second.insert(std::make_pair(loc.filename, isInside));
        } else {
          isInside = it->second;
        }
      }
      if (!isInside)
        continue;
      for (const auto &kblock : kfunc->blocks) {
        auto b = kblock.get();
        if (!loc.isInside(b))
          continue;
        block2location.insert(std::make_pair(b, le));

        auto it = block2targets.find(b);
        std::unordered_map<ReachWithError, ref<Target> > *targetMap;
        ref<Target> target = nullptr;
        if (it == block2targets.end()) {
          targetMap = new std::unordered_map<ReachWithError, ref<Target> >();
        } else {
          targetMap = it->second;
          auto itt = targetMap->find(error);
          if (itt != targetMap->end())
            target = itt->second;
        }
        if (target.isNull()) {
          target = new Target(b, error);
          targetMap->insert(std::make_pair(error, target));
          block2targets.insert(it, std::make_pair(b, targetMap));
        }
        targets->insert(target);
      }
      if (targets->empty())
        continue;
      (*all_locs)[i] = (*all_locs)[all_locs->size() - 1];
      all_locs->pop_back();
      loc2targets.insert(std::make_pair(le, targets));
      targets = new std::set<ref<Target> >();
    }
    delete targets;
  }
  delete all_locs;

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
  for (const auto &blockAndLoc : block2location) {
    auto expectedLocation = blockAndLoc.second;
    if (visited.find(expectedLocation) != visited.end() || expectedLocation->isReported)
      continue;
    visited.insert(expectedLocation);

    bool hasErrorTarget = false;
    for (const auto &errAndTarget : *block2targets[blockAndLoc.first]) {
      if (errAndTarget.second->shouldFailOnThisTarget()) {
        hasErrorTarget = true;
        break;
      }
    }
    if (!hasErrorTarget)
      continue;

    unsigned char confidenceRate = 0;
    if (noMoreStates) {
      confidenceRate = 100;
    } else {
      confidenceRate = 0; //TODO: [Aleksandr Misonizhnik], calculate it via KLEE coverage or something
    }
    assert(0 <= confidenceRate && confidenceRate <= 100);
    // klee_warning("%u%% False Positive at: %s", confidenceRate, expectedLocation->toString().c_str());
    klee_warning("False Positive at: %s", expectedLocation->toString().c_str());
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
  assert(target->getBlock() == state.prevPC->parent);
  auto expectedLocation = block2location[target->getBlock()];
  if (!expectedLocation->location.isTheSameAsIn(state.prevPC))
    return false;

  klee_warning("True Positive at: %s", expectedLocation->toString().c_str());
  // klee_warning("100%% True Positive at: %s", expectedLocation->toString().c_str());
  expectedLocation->isReported = true;
  return true;
}

TargetedExecutionManager::~TargetedExecutionManager() {
  for (auto p : block2targets)
    delete p.second;
}
