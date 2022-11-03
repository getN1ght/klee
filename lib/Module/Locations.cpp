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

bool isOSSeparator(char c) {
  return c == '/' || c == '\\';
}

bool Location::isInside(const FunctionInfo &info) const {
  size_t suffixSize = 0;
  int m = info.file.size() - 1, n = filename.size() - 1;
  for (;
       m >= 0 && n >= 0 && info.file[m] == filename[n];
       m--, n--)
    suffixSize++;
  return suffixSize >= 3 && (n == -1 ? m == -1 || isOSSeparator(info.file[m]) : m == -1 && isOSSeparator(filename[n]));
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
