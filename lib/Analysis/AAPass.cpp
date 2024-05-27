#include <MemoryModel/PointerAnalysis.h>
#include <SVF-LLVM/LLVMModule.h>
#include <SVF-LLVM/SVFIRBuilder.h>
#include <SVFIR/SVFType.h>
#include <Util/ExtAPI.h>

#include <WPA/Andersen.h>
#include <WPA/FlowSensitive.h>
#include <filesystem>
#include <llvm/Analysis/AliasAnalysis.h>

#include "klee/Internal/Analysis/AAPass.h"
#include "klee/Internal/Analysis/Util.h"

#include "klee/Config/config.h"

using namespace llvm;

char AAPass::ID = 0;

static RegisterPass<AAPass>
    WHOLEPROGRAMPA("AAPass", "Whole Program Pointer Analysis Pass");

AAPass::~AAPass() { delete _pta; }

bool AAPass::runOnModule(llvm::Module &module) {
  runPointerAnalysis(module, type);
  return false;
}

void AAPass::runPointerAnalysis(llvm::Module &module, std::uint32_t kind) {
  SVF::ExtAPI::setExtBcPath(SVF_EXT_API_BC);
  SVF::SVFModule *svfModule = SVF::LLVMModuleSet::buildSVFModule(module);
  SVF::SVFIRBuilder builder(svfModule);
  SVF::SVFIR *pag = builder.build();

  switch (kind) {
  case SVF::PointerAnalysis::Andersen_WPA:
    _pta = new SVF::Andersen(pag, SVF::Andersen::Andersen_WPA);
    break;
  case SVF::PointerAnalysis::AndersenSCD_WPA:
    _pta = new SVF::Andersen(pag, SVF::Andersen::AndersenSCD_WPA);
    break;
  case SVF::PointerAnalysis::AndersenSFR_WPA:
    _pta = new SVF::Andersen(pag, SVF::Andersen::AndersenSFR_WPA);
    break;
  case SVF::PointerAnalysis::AndersenWaveDiff_WPA:
    _pta = new SVF::Andersen(pag, SVF::PointerAnalysis::AndersenWaveDiff_WPA);
    break;
  case SVF::PointerAnalysis::FSSPARSE_WPA:
    _pta = new SVF::FlowSensitive(pag);
    break;
  default:
    llvm::errs() << "This pointer analysis has not been implemented yet.\n";
    break;
  }

  _pta->analyze();
}

AliasResult AAPass::alias(const Value *V1, const Value *V2) {
  AliasResult result = llvm::AliasResult::MayAlias;

  SVF::PAG *pag = _pta->getPAG();
  if (pag->hasValueNode(SVFValueFrom(V1)) &&
      pag->hasValueNode(SVFValueFrom(V2))) {

    switch (_pta->alias(SVFValueFrom(V1), SVFValueFrom(V2))) {
    case SVF::MayAlias:
      return llvm::AliasResult::MayAlias;
    case SVF::MustAlias:
      return llvm::AliasResult::MustAlias;
    case SVF::NoAlias:
      return llvm::AliasResult::NoAlias;
    case SVF::PartialAlias:
      return llvm::AliasResult::PartialAlias;
    }
  }

  return result;
}
