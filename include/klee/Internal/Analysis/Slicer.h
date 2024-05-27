#ifndef SLICER_H
#define SLICER_H

#include "klee/Config/config.h"

#include <dg/MemorySSA/MemorySSA.h>
#include <llvm/IR/Module.h>

#include "dg/llvm/LLVMDependenceGraph.h"
#include "dg/llvm/PointerAnalysis/PointerAnalysis.h"
#include "dg/tools/llvm-slicer.h"
// #include "dg/llvm/analysis/ReachingDefinitions/ReachingDefinitions.h"

#include "Cloner.h"

namespace klee {
class Slicer {
private:
  uint32_t slice_id = 0;
  bool got_slicing_criterion = true;

protected:
  llvm::Module *M;
  uint32_t opts = 0;
  std::string entryFunction;
  std::vector<std::string> criterions;
  dg::LLVMPointerAnalysis *PTA;
  dg::LLVMDependenceGraph dg;
  ::Slicer slicer;

public:
  Slicer(llvm::Module *mod, uint32_t o, std::string entryFunction,
         std::vector<std::string> criterions, dg::LLVMPointerAnalysis *llvmpta,
         Cloner *cloner);
  ~Slicer();

  int run();
  bool buildDG();
  bool mark();
  void computeEdges();
  bool slice();
  void remove_unused_from_module_rec();
  bool remove_unused_from_module();
  void make_declarations_external();
  const dg::LLVMDependenceGraph &getDG() const { return dg; }
  dg::LLVMDependenceGraph &getDG() { return dg; }
  void setSliceId(uint32_t id) { slice_id = id; }
};
} // namespace klee

#endif /* SLICER_H */
