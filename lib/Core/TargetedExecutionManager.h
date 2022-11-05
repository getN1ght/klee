//===-- TargetedExecutionManager.h ------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Class to manage everything for targeted execution mode
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_TARGETEDEXECUTIONMANAGER_H
#define KLEE_TARGETEDEXECUTIONMANAGER_H

#include "Target.h"
#include "TargetForest.h"
#include "klee/Module/KModule.h"

#include <unordered_map>

namespace klee {

class TargetedExecutionManager {
  struct TargetPreparator;

  /// Map of blocks to corresponding execution targets
  std::unordered_map<KBlock *, std::unordered_map<ReachWithError, ref<Target> > *> block2targets;

  /// Map of blocks to corresponding original code locations
  std::unordered_map<ref<Target>, LocatedEvent *, TargetHash, TargetCmp> target2location;

public:
  ~TargetedExecutionManager();

  std::vector<std::pair<KFunction *, ref<TargetForest> > > prepareTargets(KModule *kmodule, PathForest *paths);
  void stepTo(ExecutionState &state, KBlock *dst);

  /* Report for targeted static analysis mode */
  void reportFalsePositives(bool noMoreStates);

  void reportFalseNegative(ExecutionState &state);

  // Return true if report is successful
  bool reportTruePositive(ExecutionState &state, ReachWithError error);
};

} // End klee namespace

#endif /* KLEE_TARGETEDEXECUTIONMANAGER_H */
