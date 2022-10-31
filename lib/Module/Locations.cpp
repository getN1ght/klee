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

using namespace klee;
using namespace llvm;

bool Location::isTheSameAsIn(KInstruction *instr) const {
  return instr->info->line == line;
}

bool Location::isInside(const FunctionInfo &info) const {
  return info.file == filename;
}

bool Location::isInside(KBlock *block) const {
  auto first = block->getFirstInstruction()->info->line;
  if (first > line)
    return false;
  auto last = block->getLastInstruction()->info->line;
  return line <= last; // and `first <= line` from above
}

ResolvedLocations::ResolvedLocations(const KModule *kmodule, const Locations *locs) : ResolvedLocations() {
  auto infos = kmodule->infos.get();
  for (auto loc : *locs) {
    std::unordered_set<KBlock *> resolvedLocations;
    for (const auto &kfunc : kmodule->functions) {
      const auto &fi = infos->getFunctionInfo(*kfunc->function);
      if (!loc->isInside(fi))
        continue;
      for (const auto &kblock : kfunc->blocks) {
        if (loc->isInside(kblock.get()))
          resolvedLocations.insert(kblock.get());
      }
    }
    locations.emplace_back(loc, resolvedLocations);
  }
}
