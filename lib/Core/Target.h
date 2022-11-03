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
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  Target(KBlock *_block, ReachWithError error) : block(_block), error(error) {}
  explicit Target(KBlock *_block) : Target(_block, ReachWithError::None) {} //TODO: [Aleksandr Misonizhnik], I think, this constructor should be entirely deleted


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
};

struct TargetHash  {
  unsigned operator()(const ref<Target> &t) const { return t->hash(); }
};

struct TargetCmp {
  bool operator()(const ref<Target> &a, const ref<Target> &b) const {
    return a==b;
  }
};

  struct TargetsHash {
    unsigned operator()(const std::vector<ref<Target>> &t) const {
      unsigned res = t.size() * Expr::MAGIC_HASH_CONSTANT;
      for (const ref<Target> &target : t) {
        res <<= 1;
        res ^= target->hash() * Expr::MAGIC_HASH_CONSTANT;
      }
      return res;
    }
  };

  struct TargetsCmp {
    bool operator()(const std::vector<ref<Target>> &a,
                    const std::vector<ref<Target>> &b) const {
      if (a.size() != b.size()) {
        return false;
      }
      unsigned n = a.size();
      for (unsigned i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
          return false;
        }
      }
      return true;
    }
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

    typedef std::map<llvm::BasicBlock *,
                     std::map<llvm::BasicBlock *, VisitedBlocks>>
        BlocksHistory;
    typedef std::map<llvm::BasicBlock *,
                     std::map<llvm::BasicBlock *, VisitedTransitions>>
        TransitionsHistory;

  public:
    ref<Target> calculateByTransitionHistory(ExecutionState &state);
    ref<Target> calculateByBlockHistory(ExecutionState &state);

    TargetCalculator(const KModule &module,
                     CodeGraphDistance &codeGraphDistance)
        : module(module), codeGraphDistance(codeGraphDistance) {}

    void update(const ExecutionState &state);

  private:
    const KModule &module;
    CodeGraphDistance &codeGraphDistance;
    BlocksHistory blocksHistory;
    TransitionsHistory transitionsHistory;

    bool differenceIsEmpty(
        const ExecutionState &state,
        const std::map<llvm::BasicBlock *, VisitedBlocks> &history,
        KBlock *target);
    bool differenceIsEmpty(
        const ExecutionState &state,
        const std::map<llvm::BasicBlock *, VisitedTransitions> &history,
        KBlock *target);
    ref<Target> calculateBy(HistoryKind kind, ExecutionState &state);
  };
} // namespace klee

#endif /* KLEE_TARGET_H */
