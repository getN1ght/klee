//===-- Locations.cpp --------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Module/Locations.h"
#include "klee/Module/KModule.h"
#include "klee/Module/KInstruction.h"

#include <sstream>

using namespace klee;
using namespace llvm;

bool Location::isTheSameAsIn(KInstruction *instr) const {
  return instr->info->line == line;
}

bool Location::isInside(const FunctionInfo &info) const {
  int m = info.file.size();
  int n = filename.size();
  if (n < m)
    return false;
  return filename.substr(n - m, m) == info.file;
}

std::string Location::toString() const {
  std::stringstream out;
  out << filename << ":" << line;
  return out.str();
}

bool Location::isInside(KBlock *block) const {
  auto first = block->getFirstInstruction()->info->line;
  if (first > line)
    return false;
  auto last = block->getLastInstruction()->info->line;
  return line <= last; // and `first <= line` from above
}
