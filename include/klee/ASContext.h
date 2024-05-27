#ifndef KLEE_ASCONTEXT_H
#define KLEE_ASCONTEXT_H

#include "klee/ADT/Ref.h"
#include "llvm/IR/Instruction.h"

#include "klee/Internal/Analysis/Cloner.h"

#include <vector>

namespace klee {

class ExecutionState;

class ASContext {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  ASContext() {}

  ASContext(Cloner *cloner, std::vector<llvm::Instruction *> &callTrace,
            llvm::Instruction *inst);

  ASContext(ASContext &other) = default;

  bool operator==(ASContext &other);

  bool operator!=(ASContext &other);

  void dump();

private:
  llvm::Instruction *getTranslatedInst(Cloner *cloner, llvm::Instruction *inst);

  std::vector<llvm::Instruction *> trace;
};

} // namespace klee

#endif
