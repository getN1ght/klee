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

#include "klee/Support/OptionCategories.h"
#include "llvm/Support/Casting.h"
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
struct TargetHash;
struct EquivTargetCmp;
struct TargetCmp;

enum TargetCalculateBy { Default, Blocks, Transitions };

struct Target {
private:
  typedef std::unordered_set<Target *, TargetHash, EquivTargetCmp>
      EquivTargetHashSet;
  typedef std::unordered_set<Target *, TargetHash, TargetCmp> TargetHashSet;
  static EquivTargetHashSet cachedTargets;
  static TargetHashSet targets;
  KBlock *block;
  ReachWithError error;

  explicit Target(KBlock *_block, ReachWithError error)
      : block(_block), error(error) {}

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  static ref<Target> create(KBlock *_block, ReachWithError error);
  static ref<Target>
  create(KBlock *_block); // TODO: [Aleksandr Misonizhnik], I think, this
                          // constructor should be entirely deleted

  int compare(const Target &other) const {
    if (error != other.error)
      return error < other.error ? -1 : 1;
    if (block != other.block)
      return block < other.block ? -1 : 1;
    return 0;
  }

  bool operator<(const Target &other) const {
    return block < other.block || (block == other.block && error < other.error);
  }

  bool operator==(const Target &other) const {
    return block == other.block && error == other.error;
  }

  bool atReturn() const { return isa<KReturnBlock>(block); }

  KBlock *getBlock() const { return block; }
  ReachWithError getError() const { return error; }
  bool shouldFailOnThisTarget() const { return error != ReachWithError::None; }

  bool isNull() const { return block == nullptr; }

  explicit operator bool() const noexcept { return !isNull(); }

  unsigned hash() const { return reinterpret_cast<uintptr_t>(block); }

  std::string toString() const;
  ~Target();
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

  enum HistoryKind { Blocks, Transitions };

  typedef std::unordered_map<
      llvm::BasicBlock *, std::unordered_map<llvm::BasicBlock *, VisitedBlocks>>
      BlocksHistory;
  typedef std::unordered_map<
      llvm::BasicBlock *,
      std::unordered_map<llvm::BasicBlock *, VisitedTransitions>>
      TransitionsHistory;

public:
  TargetCalculator(const KModule &module, CodeGraphDistance &codeGraphDistance)
      : module(module), codeGraphDistance(codeGraphDistance) {}

  void update(const ExecutionState &state);

  ref<Target> calculate(ExecutionState &state);

private:
  const KModule &module;
  CodeGraphDistance &codeGraphDistance;
  BlocksHistory blocksHistory;
  TransitionsHistory transitionsHistory;

  bool differenceIsEmpty(
      const ExecutionState &state,
      const std::unordered_map<llvm::BasicBlock *, VisitedBlocks> &history,
      KBlock *target);
  bool differenceIsEmpty(
      const ExecutionState &state,
      const std::unordered_map<llvm::BasicBlock *, VisitedTransitions> &history,
      KBlock *target);
};
} // namespace klee

#endif /* KLEE_TARGET_H */
