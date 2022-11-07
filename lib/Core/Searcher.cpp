//===-- Searcher.cpp ------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Searcher.h"

#include "CoreStats.h"
#include "ExecutionState.h"
#include "Executor.h"
#include "MergeHandler.h"
#include "PTree.h"
#include "StatsTracker.h"
#include "Target.h"

#include "klee/ADT/DiscretePDF.h"
#include "klee/ADT/RNG.h"
#include "klee/ADT/WeightedQueue.h"
#include "klee/Statistics/Statistics.h"
#include "klee/Module/CodeGraphDistance.h"
#include "klee/Module/InstructionInfoTable.h"
#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"
#include "klee/Support/ErrorHandling.h"
#include "klee/System/Time.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#include <cassert>
#include <cmath>

using namespace klee;
using namespace llvm;


///

ExecutionState &DFSSearcher::selectState() {
  return *states.back();
}

void DFSSearcher::update(ExecutionState *current,
                         const std::vector<ExecutionState *> &addedStates,
                         const std::vector<ExecutionState *> &removedStates) {
  // insert states
  states.insert(states.end(), addedStates.begin(), addedStates.end());

  // remove states
  for (const auto state : removedStates) {
    if (state == states.back()) {
      states.pop_back();
    } else {
      auto it = std::find(states.begin(), states.end(), state);
      assert(it != states.end() && "invalid state removed");
      states.erase(it);
    }
  }
}

bool DFSSearcher::empty() {
  return states.empty();
}

void DFSSearcher::printName(llvm::raw_ostream &os) {
  os << "DFSSearcher\n";
}


///

ExecutionState &BFSSearcher::selectState() {
  return *states.front();
}

void BFSSearcher::update(ExecutionState *current,
                         const std::vector<ExecutionState *> &addedStates,
                         const std::vector<ExecutionState *> &removedStates) {
  // update current state
  // Assumption: If new states were added KLEE forked, therefore states evolved.
  // constraints were added to the current state, it evolved.
  if (!addedStates.empty() && current &&
      std::find(removedStates.begin(), removedStates.end(), current) == removedStates.end()) {
    auto pos = std::find(states.begin(), states.end(), current);
    assert(pos != states.end());
    states.erase(pos);
    states.push_back(current);
  }

  // insert states
  states.insert(states.end(), addedStates.begin(), addedStates.end());

  // remove states
  for (const auto state : removedStates) {
    if (state == states.front()) {
      states.pop_front();
    } else {
      auto it = std::find(states.begin(), states.end(), state);
      assert(it != states.end() && "invalid state removed");
      states.erase(it);
    }
  }
}

bool BFSSearcher::empty() {
  return states.empty();
}

void BFSSearcher::printName(llvm::raw_ostream &os) {
  os << "BFSSearcher\n";
}


///

RandomSearcher::RandomSearcher(RNG &rng) : theRNG{rng} {}

ExecutionState &RandomSearcher::selectState() {
  return *states[theRNG.getInt32() % states.size()];
}

void RandomSearcher::update(ExecutionState *current,
                            const std::vector<ExecutionState *> &addedStates,
                            const std::vector<ExecutionState *> &removedStates) {
  // insert states
  states.insert(states.end(), addedStates.begin(), addedStates.end());

  // remove states
  for (const auto state : removedStates) {
    auto it = std::find(states.begin(), states.end(), state);
    assert(it != states.end() && "invalid state removed");
    states.erase(it);
  }
}

bool RandomSearcher::empty() {
  return states.empty();
}

void RandomSearcher::printName(llvm::raw_ostream &os) {
  os << "RandomSearcher\n";
}


///

static unsigned int ulog2(unsigned int val) {
  if (val == 0)
    return UINT_MAX;
  if (val == 1)
    return 0;
  unsigned int ret = 0;
  while (val > 1) {
    val >>= 1;
    ret++;
  }
  return ret;
}

///

TargetedSearcher::TargetedSearcher(ref<Target> target, CodeGraphDistance &_distance)
    : states(std::make_unique<
             WeightedQueue<ExecutionState *, ExecutionStateIDCompare>>()),
      target(target), codeGraphDistance(_distance),
      distanceToTargetFunction(
          codeGraphDistance.getBackwardDistance(target->getBlock()->parent)) {}

ExecutionState &TargetedSearcher::selectState() { return *states->choose(0); }

bool TargetedSearcher::distanceInCallGraph(KFunction *kf, KBlock *kb,
                                           unsigned int &distance) {
  distance = UINT_MAX;
  const std::unordered_map<KBlock *, unsigned> &dist =
      codeGraphDistance.getDistance(kb);
  KBlock *targetBB = target->getBlock();
  KFunction *targetF = targetBB->parent;

  if (kf == targetF && dist.count(targetBB) != 0) {
    distance = 0;
    return true;
  }

  for (auto &kCallBlock : kf->kCallBlocks) {
    if (dist.count(kCallBlock) != 0) {
      KFunction *calledKFunction =
          kf->parent->functionMap[kCallBlock->calledFunction];
      if (distanceToTargetFunction.count(calledKFunction) != 0 &&
          distance > distanceToTargetFunction.at(calledKFunction) + 1) {
        distance = distanceToTargetFunction.at(calledKFunction) + 1;
      }
    }
  }
  return distance != UINT_MAX;
}

TargetedSearcher::WeightResult
TargetedSearcher::tryGetLocalWeight(ExecutionState *es, weight_type &weight,
                                    const std::vector<KBlock *> &localTargets) {
  unsigned int intWeight = es->steppedMemoryInstructions;
  KFunction *currentKF = es->pc->parent->parent;
  KBlock *currentKB = currentKF->blockMap[es->getPCBlock()];
  KBlock *prevKB = currentKF->blockMap[es->getPrevPCBlock()];
  const std::unordered_map<KBlock *, unsigned> &dist =
      codeGraphDistance.getDistance(currentKB);
  unsigned int localWeight = UINT_MAX;
  for (auto &end : localTargets) {
    if (dist.count(end) > 0) {
      unsigned int w = dist.at(end);
      localWeight = std::min(w, localWeight);
    }
  }

  if (localWeight == UINT_MAX)
    return Miss;
  if (localWeight == 0 &&
      (prevKB != currentKB || target->shouldFailOnThisTarget())) {
    return Done;
  }

  intWeight += localWeight;
  weight = ulog2(intWeight); // number on [0,32)-discrete-interval
  return Continue;
}

TargetedSearcher::WeightResult
TargetedSearcher::tryGetPreTargetWeight(ExecutionState *es, weight_type &weight) {
  KFunction *currentKF = es->pc->parent->parent;
  std::vector<KBlock *> localTargets;
  for (auto &kCallBlock : currentKF->kCallBlocks) {
    KFunction *calledKFunction =
        currentKF->parent->functionMap[kCallBlock->calledFunction];
    if (distanceToTargetFunction.count(calledKFunction) > 0) {
      localTargets.push_back(kCallBlock);
    }
  }

  if (localTargets.empty())
    return Miss;

  WeightResult res = tryGetLocalWeight(es, weight, localTargets);
  weight = weight + 32; // number on [32,64)-discrete-interval
  return res == Done ? Continue : res;
}

TargetedSearcher::WeightResult
TargetedSearcher::tryGetPostTargetWeight(ExecutionState *es, weight_type &weight) {
  KFunction *currentKF = es->pc->parent->parent;
  std::vector<KBlock *> &localTargets = currentKF->returnKBlocks;

  if (localTargets.empty())
    return Miss;

  WeightResult res = tryGetLocalWeight(es, weight, localTargets);
  weight = weight + 32; // number on [32,64)-discrete-interval
  return res == Done ? Continue : res;
}

TargetedSearcher::WeightResult
TargetedSearcher::tryGetTargetWeight(ExecutionState *es, weight_type &weight) {
  std::vector<KBlock *> localTargets = {target->getBlock()};
  WeightResult res = tryGetLocalWeight(es, weight, localTargets);
  return res;
}

TargetedSearcher::WeightResult
TargetedSearcher::tryGetWeight(ExecutionState *es, weight_type &weight) {
  if (target->atReturn() && !target->shouldFailOnThisTarget()) {
    if (es->prevPC->parent == target->getBlock() &&
        es->prevPC == target->getBlock()->getLastInstruction()) {
      return Done;
    } else if (es->pc->parent == target->getBlock()) {
      weight = 0;
      return Continue;
    }
  }

  BasicBlock *bb = es->getPCBlock();
  KBlock *kb = es->pc->parent->parent->blockMap[bb];
  KInstruction *ki = es->pc;
  if (kb->numInstructions && kb->getFirstInstruction() != ki &&
      states->tryGetWeight(es, weight)) {
    return Continue;
  }
  unsigned int minCallWeight = UINT_MAX, minSfNum = UINT_MAX, sfNum = 0;
  for (auto sfi = es->stack.rbegin(), sfe = es->stack.rend(); sfi != sfe;
       sfi++) {
    unsigned callWeight;
    if (distanceInCallGraph(sfi->kf, kb, callWeight)) {
      callWeight *= 2;
      callWeight += sfNum;
      if (callWeight < minCallWeight) {
        minCallWeight = callWeight;
        minSfNum = sfNum;
      }
    }

    if (sfi->caller) {
      kb = sfi->caller->parent;
      bb = kb->basicBlock;
    }
    sfNum++;

    if (minCallWeight < sfNum)
      break;
  }

  WeightResult res = Miss;
  if (minCallWeight == 0)
    res = tryGetTargetWeight(es, weight);
  else if (minSfNum == 0)
    res = tryGetPreTargetWeight(es, weight);
  else if (minSfNum != UINT_MAX)
    res = tryGetPostTargetWeight(es, weight);
  if (Done == res && target->shouldFailOnThisTarget()) {
    if (es->error != target->getError()) {
      res = Continue;
    }
  }
  return res;
}

void TargetedSearcher::update(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates) {
  weight_type weight = 0;

  // update current
  if (current && std::find(removedStates.begin(), removedStates.end(),
                           current) == removedStates.end()) {
    switch (tryGetWeight(current, weight)) {
    case Continue:
      states->update(current, weight);
      break;
    case Done:
      reachedOnLastUpdate.push_back(current);
      break;
    case Miss:
      current->targetForest.remove(target);
      states->remove(current);
      break;
    }
  }

  // insert states
  for (const auto state : addedStates) {
    switch (tryGetWeight(state, weight)) {
    case Continue:
      states->insert(state, weight);
      break;
    case Done:
      states->insert(state, weight);
      reachedOnLastUpdate.push_back(state);
      break;
    case Miss:
      state->targetForest.remove(target);
      break;
    }
  }

  // remove states
  for (const auto state : removedStates) {
    if (target->atReturn() && !target->shouldFailOnThisTarget() &&
        state->prevPC == target->getBlock()->getLastInstruction())
      reachedOnLastUpdate.push_back(state);
    else {
      switch (tryGetWeight(state, weight)) {
      case Done:
        reachedOnLastUpdate.push_back(state);
        break;
      case Miss:
        state->targetForest.remove(target);
        states->remove(state);
        break;
      case Continue:
        states->remove(state);
        break;
      }
    }
  }
}

bool TargetedSearcher::empty() { return states->empty(); }

void TargetedSearcher::printName(llvm::raw_ostream &os) {
  os << "TargetedSearcher";
}

TargetedSearcher::~TargetedSearcher() {
  while (!states->empty()) {
    auto &state = selectState();
    state.targetForest.remove(target);
    states->remove(&state);
  }
}

std::vector<ExecutionState *> TargetedSearcher::reached() {
  return reachedOnLastUpdate;
}

void TargetedSearcher::removeReached() {
  for (auto state : reachedOnLastUpdate) {
    states->remove(state);
    state->targetForest.stepTo(target);
  }
  reachedOnLastUpdate.clear();
}

///

GuidedSearcher::GuidedSearcher(
    Searcher *baseSearcher, CodeGraphDistance &codeGraphDistance,
    TargetCalculator &stateHistory,
    std::set<ExecutionState *, ExecutionStateIDCompare> &pausedStates,
    std::size_t bound, RNG &rng)
    : guidance(CoverageGuidance), baseSearcher(baseSearcher),
      codeGraphDistance(codeGraphDistance), stateHistory(&stateHistory),
      pausedStates(pausedStates), bound(bound), theRNG(rng) {}

GuidedSearcher::GuidedSearcher(
    CodeGraphDistance &codeGraphDistance,
    std::set<ExecutionState *, ExecutionStateIDCompare> &pausedStates,
    std::size_t bound, RNG &rng)
    : guidance(ErrorGuidance), baseSearcher(nullptr),
      codeGraphDistance(codeGraphDistance), stateHistory(nullptr),
      pausedStates(pausedStates), bound(bound), theRNG(rng) {}

ExecutionState &GuidedSearcher::selectState() {
  unsigned size = historiesAndTargets.size();
  index = theRNG.getInt32() % (size + 1);
  ExecutionState *state = nullptr;
  if (CoverageGuidance == guidance && index == size) {
    assert(baseSearcher);
    state = &baseSearcher->selectState();
  } else {
    index = index % size;
    auto &historyTargetPair = historiesAndTargets[index];
    ref<TargetForest::History> history = historyTargetPair.first;
    ref<Target> target = historyTargetPair.second;
    assert(targetedSearchers.find(history) != targetedSearchers.end() &&
           targetedSearchers.at(history).find(target) != targetedSearchers.at(history).end() &&
           !targetedSearchers.at(history)[target]->empty());
    state = &targetedSearchers.at(history).at(target)->selectState();
  }
  targetForestInfos.clear();
  return *state;
}

bool GuidedSearcher::isStuck(ExecutionState &state) {
  KInstruction *prevKI = state.prevPC;
  return (prevKI->inst->isTerminator() &&
          state.multilevel.count(state.getPCBlock()) > bound);
}

void GuidedSearcher::innerUpdate(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates) {
  std::vector<ExecutionState *> baseAddedStates(addedStates.begin(),
                                                     addedStates.end());
  std::vector<ExecutionState *> baseRemovedStates(removedStates.begin(),
                                                       removedStates.end());

  if (ErrorGuidance == guidance) {
    if (current && isStuck(*current)) {
      baseRemovedStates.push_back(current);
    }
    for (const auto state : addedStates) {
      if (isStuck(*state)) {
        pausedStates.insert(state);
        auto is = std::find(baseAddedStates.begin(),
                            baseAddedStates.end(), state);
        if (is != baseAddedStates.end()) {
          baseAddedStates.erase(is);
        } else {
          baseRemovedStates.push_back(state);
        }
      }
    }
  }

  for (const auto state : baseAddedStates) {
    if (!state->targetForest.empty()) {
      for (auto &targetF : state->targetForest) {
        auto target = targetF.first;
        assert(target && "Target shoud be not null!");
        targetForestInfos[state->targetForest.getHistory()].insert(target);
        addedTStates[state->targetForest.getHistory()][target].push_back(state);
      }
    } else {
      targetlessStates.push_back(state);
    }
  }

  for (const auto state : baseRemovedStates) {
    for (auto &targetF : state->targetForest) {
      auto target = targetF.first;
      assert(target && "Target shoud be not null!");
      targetForestInfos[state->targetForest.getHistory()].insert(target);
      removedTStates[state->targetForest.getHistory()][target].push_back(state);
    }
  }

  ref<TargetForest::History> currTargetHistory;
  std::set<ref<Target>> currTargets;
  if (current) {
    currTargetHistory = current->targetForest.getHistory();
    for (auto &targetF : current->targetForest) {
      auto target = targetF.first;
      assert(target && "Target shoud be not null!");
      currTargets.insert(target);
    }
  }

  if (current && !currTargets.empty()) {
    for (auto &targetF : current->targetForest) {
      auto target = targetF.first;
      assert(target && "Target shoud be not null!");
      targetForestInfos[current->targetForest.getHistory()].insert(target);
    }
  } else if (current &&
             std::find(baseRemovedStates.begin(), baseRemovedStates.end(),
                       current) == baseRemovedStates.end()) {
    targetlessStates.push_back(current);
  }

  if (!baseRemovedStates.empty()) {
    std::vector<ExecutionState *> alt = baseRemovedStates;
    for (const auto state : alt) {
      auto it = pausedStates.find(state);
      if (it != pausedStates.end()) {
        pausedStates.erase(it);
        baseRemovedStates.erase(
            std::remove(baseRemovedStates.begin(),
                        baseRemovedStates.end(), state),
            baseRemovedStates.end());
      }
    }
  }

  if (CoverageGuidance == guidance) {
    assert(stateHistory);
    for (const auto state : targetlessStates) {
      if (isStuck(*state)) {
        ref<Target> target(stateHistory->calculateByBlockHistory(*state));
        if (target) {
          state->targetForest.add(target);
          targetForestInfos[state->targetForest.getHistory()].insert(target);
          addedTStates[state->targetForest.getHistory()][target].push_back(state);
        } else {
          pausedStates.insert(state);
          if (std::find(baseAddedStates.begin(), baseAddedStates.end(), state) !=
              baseAddedStates.end()) {
            auto is = std::find(baseAddedStates.begin(), baseAddedStates.end(),
                                state);
            baseAddedStates.erase(is);
          } else {
            baseRemovedStates.push_back(state);
          }
        }
      }
    }
  }
  targetlessStates.clear();

  for (auto &history : targetForestInfos) {
    auto &historiedTargetedSearchers = targetedSearchers[history.first];
    for (auto &target : history.second) {
      ExecutionState *currTState =
          currTargets.count(target) != 0 ? current : nullptr;

      if (historiedTargetedSearchers.count(target) != 0 ||
          tryAddTarget(history.first, target)) {

        historiedTargetedSearchers[target]->update(
            currTState, addedTStates[history.first][target],
            removedTStates[history.first][target]);
        if (historiedTargetedSearchers[target]->empty()) {
          removeTarget(history.first, target);
        }
      } else if (isReached(history.first, target)) {
        assert(removedTStates[history.first][target].empty());
        for (auto &state : addedTStates[history.first][target]) {
          state->targetForest.remove(target);
        }
      }
      addedTStates[history.first][target].clear();
      removedTStates[history.first][target].clear();
    }
    if (targetedSearchers[history.first].empty())
      targetedSearchers.erase(history.first);
  }
  targetForestInfos.clear();

  if (CoverageGuidance == guidance) {
    assert(baseSearcher);
    baseSearcher->update(current, baseAddedStates, baseRemovedStates);
  }
}

void GuidedSearcher::update(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates,
    TargetToStateSetMap &reachedStates) {
  innerUpdate(current, addedStates, removedStates);
  collectReached(reachedStates);
  clearReached(removedStates);
}

void GuidedSearcher::update(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates) {
  innerUpdate(current, addedStates, removedStates);
  clearReached(removedStates);
}

void GuidedSearcher::collectReached(TargetToStateSetMap &reachedStates) {
  for (auto &historyTarget : historiesAndTargets) {
    auto &history = historyTarget.first;
    auto &target = historyTarget.second;
    auto reached =
        targetedSearchers.at(history).at(target)->reached();
    if (!reached.empty()) {
      for (auto state : reached)
        reachedStates[target].insert(state);
    }
  }
}

void GuidedSearcher::clearReached(const std::vector<ExecutionState *> &removedStates) {
  TargetForestHisoryTargetVector::iterator it = historiesAndTargets.begin();
  TargetForestHisoryTargetVector::iterator ie = historiesAndTargets.end();
  for (; it != ie;) {
    auto &history = it->first;
    auto &target = it->second;
      auto reached = targetedSearchers.at(history).at(target)->reached();

      for (auto &state : reached) {
        if (std::find(removedStates.begin(), removedStates.end(), state) ==
            removedStates.end()) {

          for (auto &targetF : state->targetForest) {
            auto anotherTarget = targetF.first;
            assert(anotherTarget && "Target shoud be not null!");
            if (target != anotherTarget) {
              targetForestInfos[history].insert(anotherTarget);
              removedTStates[history][anotherTarget].push_back(state);
            }
          }
        }
      }

      if (!reached.empty()) {
        if (target->shouldFailOnThisTarget()) {
          reachedTargets[history].insert(target);
        }
        targetedSearchers.at(history).at(target)->removeReached();
        if (CoverageGuidance == guidance ||
            (ErrorGuidance == guidance && target->shouldFailOnThisTarget()) ||
            targetedSearchers.at(history).at(target)->empty()) {
          it = removeTarget(history, target);
          ie = historiesAndTargets.end();
        } else {
          ++it;
        }
      } else {
        ++it;
      }

      for (auto &state : reached) {
        for (auto &targetF : state->targetForest) {
          auto anotherTarget = targetF.first;
          assert(anotherTarget && "Target shoud be not null!");
          targetForestInfos[state->targetForest.getHistory()].insert(
              anotherTarget);
          addedTStates[state->targetForest.getHistory()][anotherTarget]
              .push_back(state);
        }
      }
      if (targetedSearchers.at(history).empty())
        targetedSearchers.erase(history);
  }

  for (auto &history : targetForestInfos) {
    if (targetedSearchers.count(history.first) == 0) {
      targetedSearchers.insert(
        std::make_pair(history.first, TargetHashMap<std::unique_ptr<TargetedSearcher>>()));
    }
    for (auto &target : history.second) {
      if (targetedSearchers[history.first].count(target) != 0 ||
          tryAddTarget(history.first, target)) {
        targetedSearchers[history.first][target]->update(
            nullptr, addedTStates[history.first][target],
            removedTStates[history.first][target]);
        if (targetedSearchers[history.first][target]->empty()) {
          removeTarget(history.first, target);
        }
      } else if (isReached(history.first, target)) {
        assert(removedTStates[history.first][target].empty());
        for (auto &state : addedTStates[history.first][target]) {
          state->targetForest.remove(target);
        }
      }
      addedTStates[history.first][target].clear();
      removedTStates[history.first][target].clear();
    }

    if (targetedSearchers[history.first].empty())
      targetedSearchers.erase(history.first);
  }
  targetForestInfos.clear();
}

bool GuidedSearcher::empty() {
  return CoverageGuidance == guidance ? baseSearcher->empty()
                                      : targetedSearchers.empty();
}

void GuidedSearcher::printName(llvm::raw_ostream &os) {
  os << "GuidedSearcher\n";
}

bool GuidedSearcher::isReached(ref<TargetForest::History> history, ref<Target> target) {
  return reachedTargets.count(history) != 0 &&
         reachedTargets[history].count(target) != 0;
}

bool GuidedSearcher::tryAddTarget(ref<TargetForest::History> history,
                                  ref<Target> target) {
  if (isReached(history, target)) {
    return false;
  }
  targetedSearchers[history][target] =
      std::make_unique<TargetedSearcher>(target, codeGraphDistance);
  historiesAndTargets.push_back({history, target});
  return true;
}

GuidedSearcher::TargetForestHisoryTargetVector::iterator
GuidedSearcher::removeTarget(ref<TargetForest::History> history,
                             ref<Target> target) {
  targetedSearchers.at(history).erase(target);
  std::pair<ref<TargetForest::History>, ref<Target>> htp = {history, target};
  auto it = std::find(historiesAndTargets.begin(), historiesAndTargets.end(), htp);
  assert(it != historiesAndTargets.end());
  return historiesAndTargets.erase(it);
}

///

WeightedRandomSearcher::WeightedRandomSearcher(WeightType type, RNG &rng)
  : states(std::make_unique<DiscretePDF<ExecutionState*, ExecutionStateIDCompare>>()),
    theRNG{rng},
    type(type) {

  switch(type) {
  case Depth:
  case RP:
    updateWeights = false;
    break;
  case InstCount:
  case CPInstCount:
  case QueryCost:
  case MinDistToUncovered:
  case CoveringNew:
    updateWeights = true;
    break;
  default:
    assert(0 && "invalid weight type");
  }
}

ExecutionState &WeightedRandomSearcher::selectState() {
  return *states->choose(theRNG.getDoubleL());
}

double WeightedRandomSearcher::getWeight(ExecutionState *es) {
  switch(type) {
    default:
    case Depth:
      return es->depth;
    case RP:
      return std::pow(0.5, es->depth);
    case InstCount: {
      uint64_t count = theStatisticManager->getIndexedValue(stats::instructions,
                                                            es->pc->info->id);
      double inv = 1. / std::max((uint64_t) 1, count);
      return inv * inv;
    }
    case CPInstCount: {
      StackFrame &sf = es->stack.back();
      uint64_t count = sf.callPathNode->statistics.getValue(stats::instructions);
      double inv = 1. / std::max((uint64_t) 1, count);
      return inv;
    }
    case QueryCost:
      return (es->queryMetaData.queryCost.toSeconds() < .1)
                 ? 1.
                 : 1. / es->queryMetaData.queryCost.toSeconds();
    case CoveringNew:
    case MinDistToUncovered: {
      uint64_t md2u = computeMinDistToUncovered(es->pc,
                                                es->stack.back().minDistToUncoveredOnReturn);

      double invMD2U = 1. / (md2u ? md2u : 10000);
      if (type == CoveringNew) {
        double invCovNew = 0.;
        if (es->instsSinceCovNew)
          invCovNew = 1. / std::max(1, (int) es->instsSinceCovNew - 1000);
        return (invCovNew * invCovNew + invMD2U * invMD2U);
      } else {
        return invMD2U * invMD2U;
      }
    }
  }
}

void WeightedRandomSearcher::update(ExecutionState *current,
                                    const std::vector<ExecutionState *> &addedStates,
                                    const std::vector<ExecutionState *> &removedStates) {

  // update current
  if (current && updateWeights &&
      std::find(removedStates.begin(), removedStates.end(), current) == removedStates.end())
    states->update(current, getWeight(current));

  // insert states
  for (const auto state : addedStates)
    states->insert(state, getWeight(state));

  // remove states
  for (const auto state : removedStates)
    states->remove(state);
}

bool WeightedRandomSearcher::empty() {
  return states->empty();
}

void WeightedRandomSearcher::printName(llvm::raw_ostream &os) {
  os << "WeightedRandomSearcher::";
  switch(type) {
    case Depth              : os << "Depth\n"; return;
    case RP                 : os << "RandomPath\n"; return;
    case QueryCost          : os << "QueryCost\n"; return;
    case InstCount          : os << "InstCount\n"; return;
    case CPInstCount        : os << "CPInstCount\n"; return;
    case MinDistToUncovered : os << "MinDistToUncovered\n"; return;
    case CoveringNew        : os << "CoveringNew\n"; return;
    default                 : os << "<unknown type>\n"; return;
  }
}


///

// Check if n is a valid pointer and a node belonging to us
#define IS_OUR_NODE_VALID(n)                                                   \
  (((n).getPointer() != nullptr) && (((n).getInt() & idBitMask) != 0))

RandomPathSearcher::RandomPathSearcher(PTree &processTree, RNG &rng)
  : processTree{processTree},
    theRNG{rng},
    idBitMask{processTree.getNextId()} {};

ExecutionState &RandomPathSearcher::selectState() {
  unsigned flips=0, bits=0;
  assert(processTree.root.getInt() & idBitMask && "Root should belong to the searcher");
  PTreeNode *n = processTree.root.getPointer();
  while (!n->state) {
    if (!IS_OUR_NODE_VALID(n->left)) {
      assert(IS_OUR_NODE_VALID(n->right) && "Both left and right nodes invalid");
      assert(n != n->right.getPointer());
      n = n->right.getPointer();
    } else if (!IS_OUR_NODE_VALID(n->right)) {
      assert(IS_OUR_NODE_VALID(n->left) && "Both right and left nodes invalid");
      assert(n != n->left.getPointer());
      n = n->left.getPointer();
    } else {
      if (bits==0) {
        flips = theRNG.getInt32();
        bits = 32;
      }
      --bits;
      n = ((flips & (1U << bits)) ? n->left : n->right).getPointer();
    }
  }

  return *n->state;
}

void RandomPathSearcher::update(ExecutionState *current,
                                const std::vector<ExecutionState *> &addedStates,
                                const std::vector<ExecutionState *> &removedStates) {
  // insert states
  for (auto es : addedStates) {
    PTreeNode *pnode = es->ptreeNode, *parent = pnode->parent;
    PTreeNodePtr *childPtr;

    childPtr = parent ? ((parent->left.getPointer() == pnode) ? &parent->left
                                                              : &parent->right)
                      : &processTree.root;
    while (pnode && !IS_OUR_NODE_VALID(*childPtr)) {
      childPtr->setInt(childPtr->getInt() | idBitMask);
      pnode = parent;
      if (pnode)
        parent = pnode->parent;

      childPtr = parent
                     ? ((parent->left.getPointer() == pnode) ? &parent->left
                                                             : &parent->right)
                     : &processTree.root;
    }
  }

  // remove states
  for (auto es : removedStates) {
    PTreeNode *pnode = es->ptreeNode, *parent = pnode->parent;

    while (pnode && !IS_OUR_NODE_VALID(pnode->left) &&
           !IS_OUR_NODE_VALID(pnode->right)) {
      auto childPtr =
          parent ? ((parent->left.getPointer() == pnode) ? &parent->left
                                                         : &parent->right)
                 : &processTree.root;
      assert(IS_OUR_NODE_VALID(*childPtr) && "Removing pTree child not ours");
      childPtr->setInt(childPtr->getInt() & ~idBitMask);
      pnode = parent;
      if (pnode)
        parent = pnode->parent;
    }
  }
}

bool RandomPathSearcher::empty() {
  return !IS_OUR_NODE_VALID(processTree.root);
}

void RandomPathSearcher::printName(llvm::raw_ostream &os) {
  os << "RandomPathSearcher\n";
}


///

MergingSearcher::MergingSearcher(Searcher *baseSearcher)
  : baseSearcher{baseSearcher} {};

void MergingSearcher::pauseState(ExecutionState &state) {
  assert(std::find(pausedStates.begin(), pausedStates.end(), &state) == pausedStates.end());
  pausedStates.push_back(&state);
  baseSearcher->update(nullptr, {}, {&state});
}

void MergingSearcher::continueState(ExecutionState &state) {
  auto it = std::find(pausedStates.begin(), pausedStates.end(), &state);
  assert(it != pausedStates.end());
  pausedStates.erase(it);
  baseSearcher->update(nullptr, {&state}, {});
}

ExecutionState& MergingSearcher::selectState() {
  assert(!baseSearcher->empty() && "base searcher is empty");

  if (!UseIncompleteMerge)
    return baseSearcher->selectState();

  // Iterate through all MergeHandlers
  for (auto cur_mergehandler: mergeGroups) {
    // Find one that has states that could be released
    if (!cur_mergehandler->hasMergedStates()) {
      continue;
    }
    // Find a state that can be prioritized
    ExecutionState *es = cur_mergehandler->getPrioritizeState();
    if (es) {
      return *es;
    } else {
      if (DebugLogIncompleteMerge){
        llvm::errs() << "Preemptively releasing states\n";
      }
      // If no state can be prioritized, they all exceeded the amount of time we
      // are willing to wait for them. Release the states that already arrived at close_merge.
      cur_mergehandler->releaseStates();
    }
  }
  // If we were not able to prioritize a merging state, just return some state
  return baseSearcher->selectState();
}

void MergingSearcher::update(ExecutionState *current,
                             const std::vector<ExecutionState *> &addedStates,
                             const std::vector<ExecutionState *> &removedStates) {
  // We have to check if the current execution state was just deleted, as to
  // not confuse the nurs searchers
  if (std::find(pausedStates.begin(), pausedStates.end(), current) == pausedStates.end()) {
    baseSearcher->update(current, addedStates, removedStates);
  }
}

bool MergingSearcher::empty() {
  return baseSearcher->empty();
}

void MergingSearcher::printName(llvm::raw_ostream &os) {
  os << "MergingSearcher\n";
}


///

BatchingSearcher::BatchingSearcher(Searcher *baseSearcher, time::Span timeBudget, unsigned instructionBudget)
  : baseSearcher{baseSearcher},
    timeBudget{timeBudget},
    instructionBudget{instructionBudget} {};

ExecutionState &BatchingSearcher::selectState() {
  if (!lastState ||
      (((timeBudget.toSeconds() > 0) &&
        (time::getWallTime() - lastStartTime) > timeBudget)) ||
      ((instructionBudget > 0) &&
       (stats::instructions - lastStartInstructions) > instructionBudget)) {
    if (lastState) {
      time::Span delta = time::getWallTime() - lastStartTime;
      auto t = timeBudget;
      t *= 1.1;
      if (delta > t) {
        klee_message("increased time budget from %f to %f\n", timeBudget.toSeconds(), delta.toSeconds());
        timeBudget = delta;
      }
    }
    lastState = &baseSearcher->selectState();
    lastStartTime = time::getWallTime();
    lastStartInstructions = stats::instructions;
    return *lastState;
  } else {
    return *lastState;
  }
}

void BatchingSearcher::update(ExecutionState *current,
                              const std::vector<ExecutionState *> &addedStates,
                              const std::vector<ExecutionState *> &removedStates) {
  // drop memoized state if it is marked for deletion
  if (std::find(removedStates.begin(), removedStates.end(), lastState) != removedStates.end())
    lastState = nullptr;
  // update underlying searcher
  baseSearcher->update(current, addedStates, removedStates);
}

bool BatchingSearcher::empty() {
  return baseSearcher->empty();
}

void BatchingSearcher::printName(llvm::raw_ostream &os) {
  os << "<BatchingSearcher> timeBudget: " << timeBudget
     << ", instructionBudget: " << instructionBudget
     << ", baseSearcher:\n";
  baseSearcher->printName(os);
  os << "</BatchingSearcher>\n";
}


///

IterativeDeepeningTimeSearcher::IterativeDeepeningTimeSearcher(Searcher *baseSearcher)
  : baseSearcher{baseSearcher} {};

ExecutionState &IterativeDeepeningTimeSearcher::selectState() {
  ExecutionState &res = baseSearcher->selectState();
  startTime = time::getWallTime();
  return res;
}

void IterativeDeepeningTimeSearcher::update(ExecutionState *current,
                                            const std::vector<ExecutionState *> &addedStates,
                                            const std::vector<ExecutionState *> &removedStates) {

  const auto elapsed = time::getWallTime() - startTime;

  // update underlying searcher (filter paused states unknown to underlying searcher)
  if (!removedStates.empty()) {
    std::vector<ExecutionState *> alt = removedStates;
    for (const auto state : removedStates) {
      auto it = pausedStates.find(state);
      if (it != pausedStates.end()) {
        pausedStates.erase(it);
        alt.erase(std::remove(alt.begin(), alt.end(), state), alt.end());
      }
    }    
    baseSearcher->update(current, addedStates, alt);
  } else {
    baseSearcher->update(current, addedStates, removedStates);
  }

  // update current: pause if time exceeded
  if (current &&
      std::find(removedStates.begin(), removedStates.end(), current) == removedStates.end() &&
      elapsed > time) {
    pausedStates.insert(current);
    baseSearcher->update(nullptr, {}, {current});
  }

  // no states left in underlying searcher: fill with paused states
  if (baseSearcher->empty()) {
    time *= 2U;
    klee_message("increased time budget to %f\n", time.toSeconds());
    std::vector<ExecutionState *> ps(pausedStates.begin(), pausedStates.end());
    baseSearcher->update(nullptr, ps, std::vector<ExecutionState *>());
    pausedStates.clear();
  }
}

bool IterativeDeepeningTimeSearcher::empty() {
  return baseSearcher->empty() && pausedStates.empty();
}

void IterativeDeepeningTimeSearcher::printName(llvm::raw_ostream &os) {
  os << "IterativeDeepeningTimeSearcher\n";
}


///

InterleavedSearcher::InterleavedSearcher(const std::vector<Searcher*> &_searchers) {
  searchers.reserve(_searchers.size());
  for (auto searcher : _searchers)
    searchers.emplace_back(searcher);
}

ExecutionState &InterleavedSearcher::selectState() {
  Searcher *s = searchers[--index].get();
  if (index == 0) index = searchers.size();
  return s->selectState();
}

void InterleavedSearcher::update(ExecutionState *current,
                                 const std::vector<ExecutionState *> &addedStates,
                                 const std::vector<ExecutionState *> &removedStates) {

  // update underlying searchers
  for (auto &searcher : searchers)
    searcher->update(current, addedStates, removedStates);
}

bool InterleavedSearcher::empty() {
  return searchers[0]->empty();
}

void InterleavedSearcher::printName(llvm::raw_ostream &os) {
  os << "<InterleavedSearcher> containing " << searchers.size() << " searchers:\n";
  for (const auto &searcher : searchers)
    searcher->printName(os);
  os << "</InterleavedSearcher>\n";
}
