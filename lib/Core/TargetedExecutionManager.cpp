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


std::vector<Location *> *collectLocations(std::vector<Locations *> &paths) {
  std::unordered_set<Location *> result;
  for (auto path : paths) {
    result.insert(path->start);
    if (path->isSingleton())
      continue;
    for (auto loc : *path) {
      result.insert(loc);
    }
  }
  auto out = new std::vector<Location *>();
  for (auto loc : result)
    out->push_back(loc);
  return out;
}


std::vector<std::pair<KFunction *, ref<TargetForest> > >
TargetedExecutionManager::prepareTargets(const KModule *kmodule, std::vector<Locations *> &paths) {
  auto all_locs = collectLocations(paths);

  auto infos = kmodule->infos.get();
  std::unordered_map<Location *, std::unordered_set<KBlock *> *> loc2blocks;
  for (const auto &kfunc : kmodule->functions) {
    const auto &fi = infos->getFunctionInfo(*kfunc->function);
    auto blocks = new std::unordered_set<KBlock *>();
    for (int i = all_locs->size() - 1; i >= 0; i--) {
      auto loc = (*all_locs)[i];
      if (!loc->isInside(fi))
        continue;
      for (const auto &kblock : kfunc->blocks) {
        if (loc->isInside(kblock.get()))
          blocks->insert(kblock.get());
      }
      if (blocks->empty())
        continue;
      (*all_locs)[i] = (*all_locs)[all_locs->size() - 1];
      all_locs->pop_back();
      loc2blocks.insert(std::make_pair(loc, blocks));
      blocks = new std::unordered_set<KBlock *>();
    }
    delete blocks;
  }
  delete all_locs;

  std::unordered_map<KFunction *, std::vector<Locations *> > pathForest;
  for (auto locs : paths) {
    auto kf = (*loc2blocks[locs->start]->begin())->parent;
    auto it = pathForest.find(kf);
    if (it == pathForest.end()) {
      std::vector<Locations *> path{locs};
      pathForest.insert(it, std::make_pair(kf, path));
    } else {
      it->second.push_back(locs);
    }

    auto targetError = locs->targetError();
    auto itl = locs->begin();
    auto itle = locs->end();
    auto itllast = std::prev(itle);
    for (; itl != itle; itl++) {
      auto originalLoc = *itl;
      auto blocks = loc2blocks[originalLoc];
      for (auto b : *blocks) {
        auto error = itl == itllast ? targetError : ReachWithError::None;
        auto targets = block2targets.find(b);
        std::unordered_map<ReachWithError, ref<Target> > *targetMap;
        if (targets == block2targets.end()) {
          targetMap = new std::unordered_map<ReachWithError, ref<Target> >();
        } else {
          targetMap = targets->second;
          if (targetMap->find(error) != targetMap->end())
            continue;
        }
        targetMap->insert(std::make_pair(error, new Target(b, error)));
        block2targets.insert(targets, std::make_pair(b, targetMap));
        block2location.insert(std::make_pair(b, originalLoc));
      }
    }
  }

  std::vector<std::pair<KFunction *, ref<TargetForest> > > whitelists;
  for (const auto &funcAndPaths : pathForest) {
    ref<TargetForest> whitelist = new TargetForest(funcAndPaths.second, loc2blocks, block2targets);
    whitelists.emplace_back(funcAndPaths.first, whitelist);
  }
  return whitelists;
}

bool TargetedExecutionManager::stepTo(ExecutionState &state, KBlock *dst) {
  if (isa<KReturnBlock>(dst))
    return false;
  auto it = block2targets.find(dst);
  if (it == block2targets.end())
    return false;
  state.targetsOfCurrentKBlock = it->second;
  return false;
}

void TargetedExecutionManager::reportFalsePositives(bool noMoreStates) {
  std::unordered_set<Location *> visited;
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
    // klee_warning("%u\% False Positive at: %s", confidenceRate, expectedLocation->toString().c_str());
    klee_warning("False Positive at: %s", expectedLocation->toString().c_str());
  }
}

void TargetedExecutionManager::reportFalseNegative(ExecutionState &state) {
  auto info = state.prevPC->info;
  std::ostringstream out;
  out << "100%% False Negative at: " << info->file << ':' << info->line << ':' << info->column;
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
  if (!expectedLocation->isTheSameAsIn(state.prevPC))
    return false;

  klee_warning("100%% True Positive at: %s", expectedLocation->toString().c_str());
  expectedLocation->isReported = true;
  return true;
}

TargetedExecutionManager::~TargetedExecutionManager() {}
