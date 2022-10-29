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
#include "klee/Module/InstructionInfoTable.h"
#include "klee/Module/KInstruction.h"

using namespace klee;
using namespace llvm;


ResolvedLocations::ResolvedLocations(const KModule *kmodule, const Locations &locs) : ResolvedLocations() {
  const auto &infos = kmodule->infos;
  for (const auto &loc : locs) {
    const auto &file = loc.first;
    auto line = loc.second;
    std::unordered_set<KBlock *> resolvedLocations;
    for (const auto &kfunc : kmodule->functions) {
      const auto &fi = infos->getFunctionInfo(*kfunc->function);
      if (fi.file != file)
        continue;
      for (const auto &kblock : kfunc->blocks) {
        auto first = kblock->getFirstInstruction()->info->line;
        if (first > line)
          continue;
        auto last = kblock->getLastInstruction()->info->line;
        if (line <= last) { // and `first <= line` from above
          resolvedLocations.insert(kblock.get());
        }
      }
    }
    locations.push_back(resolvedLocations);
  }
}
