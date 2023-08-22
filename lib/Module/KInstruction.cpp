//===-- KInstruction.cpp --------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS
#include "klee/Support/ErrorHandling.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DebugInfoMetadata.h"
DISABLE_WARNING_POP

#include <string>

using namespace llvm;
using namespace klee;

/***/

KInstruction::~KInstruction() { delete[] operands; }

size_t KInstruction::getLine() const {
  auto &f =
      parent->parent->parent->infos->getFunctionInfo(*parent->parent->function);
  auto locationInfo = getLocationInfo(*inst, &f);
  return locationInfo.line;
}

size_t KInstruction::getColumn() const {
  auto &f =
      parent->parent->parent->infos->getFunctionInfo(*parent->parent->function);
  auto locationInfo = getLocationInfo(*inst, &f);
  return locationInfo.column;
}

// TODO problem files with same name
std::string KInstruction::getSourceFilepath() const {
  auto &f =
      parent->parent->parent->infos->getFunctionInfo(*parent->parent->function);
  auto locationInfo = getLocationInfo(*inst, &f);
  return locationInfo.file;
}
std::string KInstruction::getSourceLocationString() const {
  std::string filePath = getSourceFilepath();
  if (!filePath.empty()) {
    // TODO change format to file:line:column
    return filePath + ":" + std::to_string(getLine()) + " " +
           std::to_string(getColumn());
  } else {
    return "[no debug info]";
  }
}

std::string KInstruction::toString() const {
  return llvm::utostr(getIndex()) + " at " + parent->toString() + " (" +
         inst->getOpcodeName() + ")";
}

unsigned KInstruction::getIndex() const { return index; }

unsigned KInstruction::getDest() const {
  return parent->parent->numArgs + index +
         (parent->instructions - parent->parent->instructions);
}
