//===-- Target.h ------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_TARGET_H
#define KLEE_TARGET_H

#include "PTree.h"

#include "klee/ADT/RNG.h"
#include "klee/Module/KModule.h"
#include "klee/Module/Locations.h"
#include "klee/System/Time.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>

namespace klee {
class CodeGraphDistance;
class ExecutionState;
class Executor;

struct Target {
private:
  KBlock *block;
  ReachWithError error;

public:
  Target(KBlock *_block, ReachWithError error) : block(_block), error(error) {}
  explicit Target(KBlock *_block) : Target(_block, ReachWithError::None) {} //TODO: [Aleksandr Misonizhnik], I think, this constructor should be entirely deleted

  bool operator<(const Target &other) const {
    return block < other.block || (block == other.block && error < other.error);
  }

  bool operator==(const Target &other) const {
    return block == other.block && error == other.error;
  }

  bool atReturn() const { return isa<KReturnBlock>(block); }

  KBlock *getBlock() const { return block; }
  ReachWithError getError() const { return error; }
  bool shouldStopOnThisTarget() const { return error != ReachWithError::None; }

  bool isNull() const { return block == nullptr; }

  explicit operator bool() const noexcept { return !isNull(); }

  std::string toString() const;
};

typedef std::pair<llvm::BasicBlock *, llvm::BasicBlock *> Transition;

struct TransitionHash {
  std::size_t operator()(const Transition &p) const {
    return reinterpret_cast<size_t>(p.first) * 31 +
           reinterpret_cast<size_t>(p.second);
  }
};

class TargetCalculator {
  typedef std::unordered_set<llvm::BasicBlock *> VisitedBlocks;
  typedef std::unordered_set<Transition, TransitionHash> VisitedTransitions;
  
  enum HistoryKind {
    Blocks,
    Transitions
  };

  typedef std::map<llvm::BasicBlock *,
                   std::map<llvm::BasicBlock *, VisitedBlocks>>
      BlocksHistory;
  typedef std::map<llvm::BasicBlock *,
                   std::map<llvm::BasicBlock *, VisitedTransitions>>
      TransitionsHistory;

public:
  Target calculateByTransitionHistory(ExecutionState &state);
  Target calculateByBlockHistory(ExecutionState &state);

  TargetCalculator(const KModule &module, CodeGraphDistance &codeGraphDistance)
      : module(module), codeGraphDistance(codeGraphDistance) {}

  void update(const ExecutionState &state);

private:
  const KModule &module;
  CodeGraphDistance &codeGraphDistance;
  BlocksHistory blocksHistory;
  TransitionsHistory transitionsHistory;

  bool differenceIsEmpty(
    const ExecutionState &state, const std::map<llvm::BasicBlock *, VisitedBlocks> &history,
    KBlock *target);
  bool differenceIsEmpty(
    const ExecutionState &state, const std::map<llvm::BasicBlock *, VisitedTransitions> &history,
    KBlock *target);
  Target calculateBy(HistoryKind kind, ExecutionState &state);
};
} // namespace klee

#endif /* KLEE_TARGET_H */
