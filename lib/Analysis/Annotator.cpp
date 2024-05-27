#include <set>
#include <stdbool.h>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

#include <llvm-c/Core.h>

#include "klee/Internal/Analysis/Annotator.h"
#include "klee/Internal/Analysis/ModRefAnalysis.h"

using namespace std;
using namespace llvm;

void Annotator::annotate() {
  ModRefAnalysis::ModInfoToStoreMap &modInfoToStoreMap =
      mra->getModInfoToStoreMap();
  ModRefAnalysis::ModInfoToIdMap &modInfoToIdMap = mra->getModInfoToIdMap();

  for (ModRefAnalysis::ModInfoToStoreMap::iterator i =
           modInfoToStoreMap.begin();
       i != modInfoToStoreMap.end(); i++) {
    ModRefAnalysis::ModInfo modInfo = i->first;
    set<Instruction *> &stores = i->second;

    uint32_t sliceId = modInfoToIdMap[modInfo];
    annotateStores(stores, sliceId);
  }
}

void Annotator::annotateStores(set<Instruction *> &stores, uint32_t sliceId) {
  for (set<Instruction *>::iterator i = stores.begin(); i != stores.end();
       i++) {
    Instruction *inst = *i;
    annotateStore(inst, sliceId);
  }
}

void Annotator::annotateStore(Instruction *inst, uint32_t sliceId) {
  StoreInst *store = dyn_cast<StoreInst>(inst);
  Value *pointerOperand = store->getPointerOperand();

  /* generate a unique argument name */
  string name = string("__crit_arg_") + to_string(argId++);
  /* insert load */
  // TODO: this is probably incorrect
  LoadInst *loadInst = new LoadInst(store->getType(), pointerOperand,
                                    name.data(), (Instruction *)nullptr);
  loadInst->setAlignment(store->getAlign());
  loadInst->insertAfter(inst);

  /* create criterion function */
  Function *criterionFunction = getCriterionFunction(pointerOperand, sliceId);

  /* insert call */
  vector<Value *> args;
  args.push_back(dyn_cast<Value>(loadInst));
  CallInst *callInst = CallInst::Create(criterionFunction, args, "");
  callInst->insertAfter(loadInst);
}

Function *Annotator::getCriterionFunction(Value *pointerOperand,
                                          uint32_t sliceId) {
  PointerType *pointerType = dyn_cast<PointerType>(pointerOperand->getType());
  Type *requiredType = pointerType->getPointerElementType();

  /* get annotation info */
  AnnotationInfo &ai = annotationsMap[sliceId];

  /* search for matching functions */
  for (set<string>::iterator i = ai.fnames.begin(); i != ai.fnames.end(); i++) {
    string fname = *i;
    Function *f = module->getFunction(fname);
    assert(f);

    /* a criterion function has exactly one parameter */
    Type *argType = f->getFunctionType()->getParamType(0);
    if (argType == requiredType) {
      return f;
    }
  }

  /* generate a unique function name */
  string fname = getAnnotatedName(sliceId, ai.subId);
  module->getOrInsertFunction(fname, Type::getVoidTy(module->getContext()),
                              requiredType);

  ai.subId++;
  ai.fnames.insert(fname);

  return module->getFunction(fname);
}

string Annotator::getAnnotatedName(uint32_t sliceId, uint32_t subId) {
  return string("__crit_") + to_string(sliceId) + string("_") +
         to_string(subId);
}

set<string> &Annotator::getAnnotatedNames(uint32_t sliceId) {
  AnnotationsMap::iterator i = annotationsMap.find(sliceId);
  if (i == annotationsMap.end()) {
    /* TODO: this should not happen */
    assert(false);
  }

  return i->second.fnames;
}
