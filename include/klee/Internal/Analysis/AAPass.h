#ifndef AAPASS_H
#define AAPASS_H

#include "MemoryModel/PointerAnalysis.h"
#include <cstdint>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Pass.h>

class AAPass : public llvm::ModulePass, public llvm::AAResultBase<AAPass> {

public:
  static char ID;

  enum AliasCheckRule {
    Conservative, ///< return MayAlias if any pta says alias
    Veto,         ///< return NoAlias if any pta says no alias
    Precise       ///< return alias result by the most precise pta
  };

  AAPass()
      : llvm::ModulePass(ID), llvm::AAResultBase<AAPass>(),
        type(SVF::PointerAnalysis::Default_PTA), _pta(nullptr) {}

  ~AAPass();

  virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const {
    au.setPreservesAll();
  }

  virtual void *getAdjustedAnalysisPointer(llvm::AnalysisID id) { return this; }

  virtual llvm::AliasResult alias(const llvm::MemoryLocation &LocA,
                                  const llvm::MemoryLocation &LocB) {
    return alias(LocA.Ptr, LocB.Ptr);
  }

  virtual llvm::AliasResult alias(const llvm::Value *V1, const llvm::Value *V2);

  virtual bool runOnModule(llvm::Module &module);

  virtual inline llvm::StringRef getPassName() const { return "AAPass"; }

  void setPAType(SVF::PointerAnalysis::PTATY type) { this->type = type; }

  SVF::BVDataPTAImpl *getPTA() { return _pta; }

private:
  void runPointerAnalysis(llvm::Module &module, std::uint32_t kind);

  SVF::PointerAnalysis::PTATY type;
  SVF::BVDataPTAImpl *_pta;
};

#endif /* AAPASS_H */
