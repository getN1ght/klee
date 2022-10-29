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
  /// Map of blocks to corresponding execution targets
  std::unordered_map<KBlock *, Target *> block2target;

public:
  ~TargetedExecutionManager();

  std::vector<std::pair<KFunction *, TargetForest *> > prepareTargets(const KModule *kmodule, std::vector<Locations> &paths);
  bool stepTo(ExecutionState &state, KBlock *dst);

  /* Report for targeted static analysis mode */
  void reportFalsePositives();
  void reportFalseNegative();
  void reportTruePositive();
};

} // End klee namespace

#endif /* KLEE_TARGETEDEXECUTIONMANAGER_H */
