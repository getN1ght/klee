#ifndef SVFPOINTERANALYSIS_H
#define SVFPOINTERANALYSIS_H

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Module.h>

#include "dg/PointerAnalysis/PSNode.h"
#include "dg/llvm/PointerAnalysis/PointerAnalysis.h"

#include "AAPass.h"

class SVFPointerAnalysis {
public:
  SVFPointerAnalysis(llvm::Module *module, dg::DGLLVMPointerAnalysis *pta,
                     AAPass *aa)
      : module(module), pta(pta), aa(aa) {}

  ~SVFPointerAnalysis() {}

  void run();
  void handleVirtualCalls();
  void handleNode(dg::pta::PSNode *node);
  void handleLoad(dg::pta::PSNode *node);
  void handleStore(dg::pta::PSNode *node);
  void handleGep(dg::pta::PSNode *node);
  void handleCast(dg::pta::PSNode *node);
  void handleFuncPtr(dg::pta::PSNode *node);
  bool functionPointerCall(dg::pta::PSNode *callsite, dg::pta::PSNode *called);
  void handlePhi(dg::pta::PSNode *node);
  void handleOperand(dg::pta::PSNode *operand);
  void updatePointsTo(dg::pta::PSNode *operand, SVF::PAGNode *pagnode);
  dg::pta::PSNode *getAllocNode(SVF::ObjVar *node);
  uint64_t getAllocNodeOffset(SVF::GepObjVar *node);

  llvm::Module *module;
  dg::DGLLVMPointerAnalysis *pta;
  AAPass *aa;
};

#endif
