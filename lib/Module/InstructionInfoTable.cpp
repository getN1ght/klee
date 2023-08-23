//===-- InstructionInfoTable.cpp ------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Module/InstructionInfoTable.h"
#include "klee/Config/Version.h"

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
DISABLE_WARNING_POP

#include <cstdint>
#include <map>
#include <string>

namespace klee {

class DebugInfoExtractor {

  const llvm::Module &module;

public:
  DebugInfoExtractor(const llvm::Module &_module)
      : module(_module) {}

//  std::unique_ptr<FunctionInfo> getFunctionInfo(const llvm::Function &Func) {
//    // Fallback: Mark as unknown
//    return std::make_unique<FunctionInfo>(FunctionInfo(0));
//  }

//  std::unique_ptr<InstructionInfo>
//  getInstructionInfo(const llvm::Instruction &Inst, const FunctionInfo *f) {
//    return std::make_unique<InstructionInfo>(InstructionInfo(0));
//  }
};

// TODO need some unify with kFunction
LocationInfo getLocationInfo(const llvm::Function *func) {
  const auto dsub = func->getSubprogram();

  if (dsub != nullptr) {
    auto path = dsub->getFilename();
    return {path.str(), dsub->getLine(), 0}; // TODO why not use column here?
  }

  return {"", 0, 0};
}

// TODO need some unify with kInstruction
LocationInfo getLocationInfo(const llvm::Instruction *inst) {
  // Retrieve debug information associated with instruction
  const auto &dl = inst->getDebugLoc();

  // Check if a valid debug location is assigned to the instruction.
  if (dl.get() != nullptr) {
    auto full_path = dl->getFilename();
    auto line = dl.getLine();
    auto column = dl.getCol();

    // Still, if the line is unknown, take the context of the instruction to
    // narrow it down
    if (line == 0) {
      if (auto LexicalBlock =
              llvm::dyn_cast<llvm::DILexicalBlock>(dl.getScope())) {
        line = LexicalBlock->getLine();
        column = LexicalBlock->getColumn();
      }
    }
    return {full_path.str(), line, column};
  }

  return getLocationInfo(inst->getParent()->getParent());
}

//InstructionInfoTable::InstructionInfoTable(const llvm::Module &m) {
//  // Generate all debug instruction information
//  DebugInfoExtractor DI(m);
//
//  for (const auto &Func : m) {
//    auto F = DI.getFunctionInfo(Func);
//    auto FR = F.get();
//    functionInfos.emplace(&Func, std::move(F));
//
//    for (auto it = llvm::inst_begin(Func), ie = llvm::inst_end(Func); it != ie;
//         ++it) {
//      auto instr = &*it;
//      auto instInfo = DI.getInstructionInfo(*instr, FR);
//      auto locationInfo = getLocationInfo(instr);
////      if (withInstructions) {
////        insts[locationInfo.file][locationInfo.line][locationInfo.column].insert(
////            instr->getOpcode());
////      }
//      fileNameToFunctions[locationInfo.file].insert(&Func);
//      infos.emplace(instr, std::move(instInfo));
//    }
//  }
//
//  // Make sure that every item has a unique ID
//  size_t idCounter = 0;
////  for (auto &item : infos)
////    item.second->id = idCounter++;
////  for (auto &item : functionInfos)
////    item.second->id = idCounter++;
//}

//unsigned InstructionInfoTable::getMaxID() const {
//  return infos.size() + functionInfos.size();
//}

//const InstructionInfo &
//InstructionInfoTable::getInfo(const llvm::Instruction &inst) const {
//  auto it = infos.find(&inst);
//  if (it == infos.end())
//    llvm::report_fatal_error("invalid instruction, not present in "
//                             "initial module!");
//  return *it->second.get();
//}
//
//const FunctionInfo &
//InstructionInfoTable::getFunctionInfo(const llvm::Function &f) const {
//  auto found = functionInfos.find(&f);
//  if (found == functionInfos.end())
//    llvm::report_fatal_error("invalid instruction, not present in "
//                             "initial module!");
//
//  return *found->second.get();
//}
//
//const InstructionInfoTable::LocationToFunctionsMap &
//InstructionInfoTable::getFileNameToFunctions() const {
//  return fileNameToFunctions;
//}

//InstructionInfoTable::Instructions InstructionInfoTable::getInstructions() {
//  return std::move(insts);
//}

} // namespace klee
