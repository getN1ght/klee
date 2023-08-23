////===-- InstructionInfoTable.h ----------------------------------*- C++ -*-===//
////
////                     The KLEE Symbolic Virtual Machine
////
//// This file is distributed under the University of Illinois Open Source
//// License. See LICENSE.TXT for details.
////
////===----------------------------------------------------------------------===//

#ifndef KLEE_INSTRUCTIONINFOTABLE_H
#define KLEE_INSTRUCTIONINFOTABLE_H

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS
#include "llvm/ADT/Optional.h"
#include "llvm/Support/raw_ostream.h"
DISABLE_WARNING_POP

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace llvm {
class Function;
class Instruction;
class Module;
} // namespace llvm

namespace klee {

//// TODO move to methods of kInstruction
///// @brief InstructionInfo stores debug information for a KInstruction.
//struct InstructionInfo {
//  /// @brief The instruction id.
////  unsigned id; // TODO move to kInstruction
//  /// @brief Line number in source file.
//  //  unsigned line;
//  /// @brief Column number in source file.
//  //  unsigned column;
//  /// @brief Line number in generated assembly.ll.
////  llvm::Optional<uint64_t> assemblyLine;
//  /// @brief Source file name.
////  const std::string &file;
//
//public:
//  InstructionInfo(unsigned id) {}
//};
//
///// @brief FunctionInfo stores debug information for a KFunction.
//struct FunctionInfo { // TODO clear this too
//  /// @brief The function id.
////  unsigned id; // TODO move to kFunction
//  /// @brief Line number in source file.
////  unsigned line;
//  /// @brief Line number in generated assembly.ll.
////  llvm::Optional<uint64_t> assemblyLine;
//  /// @brief Source file name.
////  const std::string &file;
//
//public:
//  FunctionInfo(unsigned id) {}
//
//  FunctionInfo(const FunctionInfo &) = delete;
//  FunctionInfo &operator=(FunctionInfo const &) = delete;
//
//  FunctionInfo(FunctionInfo &&) = default;
//};
//
//class InstructionInfoTable {
//public:
//  using LocationToFunctionsMap =
//      std::unordered_map<std::string,
//                         std::unordered_set<const llvm::Function *>>;
//
//private:
//  std::unordered_map<const llvm::Instruction *,
//                     std::unique_ptr<InstructionInfo>> infos;
//  std::unordered_map<const llvm::Function *, std::unique_ptr<FunctionInfo>>
//      functionInfos;
//  LocationToFunctionsMap fileNameToFunctions;                // TODO remove
////  Instructions insts; // TODO remove when move prepare target to main
//
//public:
//  explicit InstructionInfoTable(
//      const llvm::Module &m);
//
////  unsigned getMaxID() const;
//  const InstructionInfo &getInfo(const llvm::Instruction &) const;
//  const FunctionInfo &getFunctionInfo(const llvm::Function &) const;
//  const LocationToFunctionsMap &getFileNameToFunctions() const;
////  Instructions getInstructions();
//};

struct LocationInfo {
  std::string file;
  size_t line;
  size_t column;
};


// TODO need unify with kFunction
LocationInfo getLocationInfo(const llvm::Function *func);

// TODO need unify with kInstruction
LocationInfo getLocationInfo(const llvm::Instruction *inst);

} // namespace klee

#endif /* KLEE_INSTRUCTIONINFOTABLE_H */
