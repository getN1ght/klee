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


std::vector<std::pair<KFunction *, ref<TargetForest> > >
TargetedExecutionManager::prepareTargets(const KModule *kmodule, std::vector<Locations *> &paths) {
  std::vector<ResolvedLocations> resolvedPaths;
  for (auto loc : paths) {
    ResolvedLocations rl(kmodule, loc);
    auto targetError = loc->targetError();
    resolvedPaths.push_back(rl);
    auto itl = rl.locations.begin();
    auto itle = rl.locations.end();
    auto itllast = std::prev(itle);
    for (; itl != itle; itl++) {
      auto originalLoc = itl->originalLocation;
      for (auto b : itl->blocks) {
        auto it = block2target.find(b);
        if (it != block2target.end())
          continue;
        auto error = itl == itllast ? targetError : ReachWithError::None;
        block2target.insert(it, std::make_pair(b, new Target(b, error)));
        block2location.insert(std::make_pair(b, originalLoc));
      }
    }
  }

  TargetForest whitelists(resolvedPaths, block2target);

  std::unordered_set<KFunction *> visited;
  std::vector<std::pair<KFunction *, ref<TargetForest> > > targets;
  for (auto &startBlockAndWhiteList : whitelists) {
    auto kf = startBlockAndWhiteList.first->getBlock()->parent;
    if (visited.count(kf) != 0)
      continue;
    visited.insert(kf);
    targets.emplace_back(kf, startBlockAndWhiteList.second);
  }
  return targets;
}

bool TargetedExecutionManager::stepTo(ExecutionState &state, KBlock *dst) {
  auto it = block2target.find(dst);
  if (it == block2target.end())
    return false;
  auto target = it->second;
  if (target->atReturn())
    return false;
  state.targetOfCurrentKBlock =
      target->shouldFailOnThisTarget() ? target : nullptr;
  return false;
}

void TargetedExecutionManager::reportFalsePositives(bool noMoreStates) {
  std::ostringstream out;
  out << (noMoreStates ? "" : "Early termination; possible ") << "False Positive at: %s";
  auto reportLine = out.str();
  std::unordered_set<Location *> visited;
  for (const auto &blockAndLoc : block2location) {
    auto expectedLocation = blockAndLoc.second;
    if (visited.find(expectedLocation) != visited.end())
      continue;
    visited.insert(expectedLocation);
    if (!expectedLocation->isReported)
      klee_warning(reportLine.c_str(), expectedLocation->toString().c_str());
  }
}

void TargetedExecutionManager::reportFalseNegative(ExecutionState &state) {
  auto info = state.prevPC->info;
  std::ostringstream out;
  out << "False Negative at: " << info->file << ':' << info->line << ':' << info->column;
  klee_warning("%s", out.str().c_str());
}

bool TargetedExecutionManager::reportTruePositive(ExecutionState &state, ReachWithError error) {
  if (!state.targetOfCurrentKBlock || state.targetOfCurrentKBlock->getError() != error)
    return false;

  assert(state.targetOfCurrentKBlock->getBlock() == state.prevPC->parent);
  auto expectedLocation = block2location[state.targetOfCurrentKBlock->getBlock()];
  if (!expectedLocation->isTheSameAsIn(state.prevPC))
    return false;

  klee_warning("True Positive at: %s", expectedLocation->toString().c_str());
  expectedLocation->isReported = true;
  return true;
}

TargetedExecutionManager::~TargetedExecutionManager() {}
