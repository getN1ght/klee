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


using namespace llvm;
using namespace klee;


std::vector<std::pair<KFunction *, TargetForest *>>
TargetedExecutionManager::prepareTargets(const KModule *kmodule, std::vector<Locations> &paths) {
  std::vector<ResolvedLocations> resolvedPaths;
  for (auto &loc : paths) {
    ResolvedLocations rl(kmodule, loc);
    auto targetError = loc.targetError();
    resolvedPaths.push_back(rl);
    auto itl = rl.locations.begin();
    auto itle = rl.locations.end();
    auto itllast = itle;
    std::advance(itllast, -1);
    for (; itl != itle; itl++) {
      for (auto b : *itl) {
        auto it = block2target.find(b);
        if (it != block2target.end())
          continue;
        auto error = itl == itllast ? targetError : ReachWithError::None;
        block2target.insert(it, std::make_pair(b, new Target(b, error)));
      }
    }
  }

  TargetForest whitelists(resolvedPaths, block2target);

  std::unordered_set<KFunction *> visited;
  std::vector<std::pair<KFunction *, TargetForest *>> targets;
  for (auto &startBlockAndWhiteList : whitelists) {
    auto kf = startBlockAndWhiteList.first->getBlock()->parent;
    if (visited.count(kf) != 0)
      continue;
    visited.insert(kf);
    targets.emplace_back(kf, startBlockAndWhiteList.second.get());
  }
  return targets;
}

bool TargetedExecutionManager::stepTo(ExecutionState &state, KBlock *dst) {
  if (state.targetOfCurrentKBlock != nullptr)
    return true; // we missed target
  auto it = block2target.find(dst);
  if (it == block2target.end())
    return false;
  auto target = it->second;
  state.whitelist.stepTo(target);
  if (target->shouldStopOnThisTarget())
    state.targetOfCurrentKBlock = target;
  return false;
}

void TargetedExecutionManager::reportFalsePositives() {
  return; //TODO: [Yurii Kostyukov], implement it
}

void TargetedExecutionManager::reportFalseNegative() {
  return; //TODO: [Yurii Kostyukov], implement it
}

void TargetedExecutionManager::reportTruePositive() {
  return; //TODO: [Yurii Kostyukov], implement it
}

TargetedExecutionManager::~TargetedExecutionManager() {
  for (auto &pair : block2target)
    delete pair.second;
}
