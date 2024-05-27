#ifndef SLICEGENERATOR_H
#define SLICEGENERATOR_H

#include <cstdint>
#include <stdbool.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include "dg/llvm/PointerAnalysis/PointerAnalysis.h"

#include "AAPass.h"
#include "Annotator.h"
#include "Cloner.h"
#include "ModRefAnalysis.h"
#include "ReachabilityAnalysis.h"

class SliceGenerator {
public:
  SliceGenerator(llvm::Module *module, ReachabilityAnalysis *ra, AAPass *aa,
                 ModRefAnalysis *mra, Cloner *cloner, llvm::raw_ostream &debugs,
                 bool lazyMode = false)
      : module(module), ra(ra), aa(aa), mra(mra), cloner(cloner),
        debugs(debugs), lazyMode(lazyMode), annotator(0), llvmpta(0) {}

  ~SliceGenerator();

  void generate();

  void generateSlice(llvm::Function *f, uint32_t sliceId,
                     ModRefAnalysis::SideEffectType type);

  void dumpSlice(llvm::Function *f, uint32_t sliceId, bool recursively = false);

private:
  void markAsSliced(llvm::Function *sliceEntry, uint32_t sliceId);

  llvm::Module *module;
  ReachabilityAnalysis *ra;
  AAPass *aa;
  ModRefAnalysis *mra;
  Cloner *cloner;
  llvm::raw_ostream &debugs;
  bool lazyMode;
  Annotator *annotator;
  dg::DGLLVMPointerAnalysis *llvmpta;
};

#endif
