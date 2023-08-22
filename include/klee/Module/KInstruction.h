//===-- KInstruction.h ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_KINSTRUCTION_H
#define KLEE_KINSTRUCTION_H

#include "klee/Config/Version.h"
#include "klee/Module/InstructionInfoTable.h"

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"
DISABLE_WARNING_POP

#include <vector>

namespace llvm {
class Instruction;
}

namespace klee {
class Executor;
struct InstructionInfo;
class KModule;
struct KBlock;

/// KInstruction - Intermediate instruction representation used
/// during execution.
struct KInstruction {
  llvm::Instruction *inst;
  const InstructionInfo *info; // TODO remove it

  /// Value numbers for each operand. -1 is an invalid value,
  /// otherwise negative numbers are indices (negated and offset by
  /// 2) into the module constant table and positive numbers are
  /// register indices.
  int *operands;
  KBlock *parent;

  // Instruction index in the basic block
  size_t index; // TODO maybe change to pointer or size_t

  /// Destination register index.
  //  unsigned dest;
public:
  /// Instruction index in the basic block
  [[nodiscard]] unsigned getIndex() const; // TODO change to uintptr_t
  /// Destination register index.
  [[nodiscard]] unsigned getDest() const; // TODO change to uintptr_t

  KInstruction() = default;               // TODO remove default constructor
  explicit KInstruction(const KInstruction &ki) = delete;
  virtual ~KInstruction();

  [[nodiscard]] size_t getLine() const;
  [[nodiscard]] size_t getColumn() const;
  [[nodiscard]] std::string getSourceFilepath() const;

  [[nodiscard]] std::string getSourceLocationString() const;
  [[nodiscard]] std::string toString() const;
};

struct KGEPInstruction : KInstruction {
  /// indices - The list of variable sized adjustments to add to the pointer
  /// operand to execute the instruction. The first element is the operand
  /// index into the GetElementPtr instruction, and the second element is the
  /// element size to multiple that index by.
  std::vector<std::pair<unsigned, uint64_t>> indices;

  /// offset - A constant offset to add to the pointer operand to execute the
  /// instruction.
  uint64_t offset;

public:
  KGEPInstruction() = default;
  explicit KGEPInstruction(const KGEPInstruction &ki) = delete;
};
} // namespace klee

#endif /* KLEE_KINSTRUCTION_H */
