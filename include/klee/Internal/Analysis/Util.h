#pragma once

#include "SVF-LLVM/LLVMModule.h"

inline SVF::SVFValue *SVFValueFrom(const llvm::Value *value) {
  return SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(value);
}

inline const llvm::Value *LLVMValueFrom(const SVF::SVFValue *value) {
  return SVF::LLVMModuleSet::getLLVMModuleSet()->getLLVMValue(value);
}
