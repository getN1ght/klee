#include <set>
#include <stack>
#include <stdio.h>
#include <vector>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include "klee/Internal/Analysis/Util.h"

#include "MemoryModel/PointerAnalysisImpl.h"
#include "klee/Internal/Analysis/AAPass.h"
#include "klee/Internal/Analysis/ReachabilityAnalysis.h"

using namespace std;
using namespace llvm;

void ReachabilityAnalysis::prepare() {
  /* remove unused functions using fixpoint */
  removeUnusedValues();
  /* compute function type map for resolving indirect calls */
  computeFunctionTypeMap();
}

void ReachabilityAnalysis::removeUnusedValues() {
  bool changed = true;
  std::set<std::string> keep;

  /* entry function must not be removed */
  keep.insert(entry);

  while (changed) {
    std::set<Function *> functions;

    for (auto &f : *module) {
      if (keep.find(f.getName().str()) != keep.end()) {
        continue;
      }

      if (f.hasNUses(0)) {
        functions.insert(&f);
      }
    }

    for (Function *f : functions) {
      debugs << "erasing: " << f->getName() << "\n";
      f->eraseFromParent();
    }

    changed = !functions.empty();
  }
}

void ReachabilityAnalysis::computeFunctionTypeMap() {
  for (auto &f : *module) {
    /* add functions which may be virtual */
    if (!isVirtual(&f)) {
      continue;
    }

    llvm::FunctionType *type = f.getFunctionType();
    functionTypeMap[type].insert(&f);

    /* if a function pointer is casted, consider the casted type as well */
    for (auto &i : f.uses()) {
      llvm::ConstantExpr *ce = llvm::dyn_cast<llvm::ConstantExpr>(&i);
      if (!ce || !ce->isCast()) {
        continue;
      }

      llvm::PointerType *pointerType =
          llvm::dyn_cast<llvm::PointerType>(ce->getType());
      if (!pointerType) {
        continue;
      }

      llvm::FunctionType *castedType = llvm::dyn_cast<llvm::FunctionType>(
          pointerType->getPointerElementType());
      if (!castedType) {
        continue;
      }

      functionTypeMap[castedType].insert(&f);
    }
  }
}

bool ReachabilityAnalysis::run(bool usePA) {
  vector<Function *> all;

  /* check parameters... */
  entryFunction = module->getFunction(entry);
  if (!entryFunction) {
    errs() << "entry function '" << entry << "' is not found\n";
    return false;
  }
  all.push_back(entryFunction);

  for (vector<string>::iterator i = targets.begin(); i != targets.end(); i++) {
    string name = *i;
    Function *f = module->getFunction(name);
    if (!f) {
      errs() << "function '" << name << "' is not found\n";
      return false;
    }
    targetFunctions.push_back(f);
    all.push_back(f);
  }

  /* build reachability map */
  for (vector<Function *>::iterator i = all.begin(); i != all.end(); i++) {
    updateReachabilityMap(*i, usePA);
  }

  /* debug */
  dumpReachableFunctions();

  return true;
}

void ReachabilityAnalysis::updateReachabilityMap(Function *f, bool usePA) {
  FunctionSet &functions = reachabilityMap[f];
  computeReachableFunctions(f, usePA, functions);
}

void ReachabilityAnalysis::computeReachableFunctions(Function *entry,
                                                     bool usePA,
                                                     FunctionSet &results) {
  std::stack<llvm::Function *> stack;
  FunctionSet pushed;

  stack.push(entry);
  pushed.insert(entry);
  results.insert(entry);

  while (!stack.empty()) {
    Function *f = stack.top();
    stack.pop();

    for (inst_iterator iter = inst_begin(f); iter != inst_end(f); iter++) {
      llvm::Instruction *inst = &*iter;
      if (inst->getOpcode() != Instruction::Call) {
        continue;
      }

      llvm::CallInst *callInst = llvm::dyn_cast<llvm::CallInst>(inst);

      /* potential call targets */
      FunctionSet targets;
      resolveCallTargets(callInst, usePA, targets);

      for (FunctionSet::iterator i = targets.begin(); i != targets.end(); i++) {
        llvm::Function *target = *i;
        results.insert(target);

        if (target->isDeclaration()) {
          continue;
        }

        if (pushed.find(target) == pushed.end()) {
          stack.push(target);
          pushed.insert(target);
        }
      }

      if (usePA) {
        updateCallMap(callInst, targets);
        updateRetMap(callInst, targets);
      }
    }
  }
}

bool ReachabilityAnalysis::isVirtual(llvm::Function *f) {
  for (auto i = f->use_begin(); i != f->use_end(); i++) {
    llvm::Value *use = *i;
    llvm::CallInst *callInst = llvm::dyn_cast<llvm::CallInst>(use);
    if (!callInst) {
      /* we found a use which is not a call instruction */
      return true;
    }

    return std::find_if(
        callInst->op_begin(), callInst->op_end(), [f](llvm::Use &arg) {
          if (auto fArg = llvm::dyn_cast<llvm::Function>(&arg)) {
            return fArg == f;
          }
          return false;
        });
  }

  return false;
}

void ReachabilityAnalysis::resolveCallTargets(llvm::CallInst *callInst,
                                              bool usePA,
                                              FunctionSet &targets) {
  llvm::Function *calledFunction = callInst->getCalledFunction();
  llvm::Value *calledValue = callInst->getCalledOperand();

  if (calledFunction != nullptr) {
    /* the called value should be one of these: function pointer, cast, alias */
    if (llvm::isa<llvm::ConstantExpr>(calledValue)) {
      Function *extracted =
          extractFunction(dyn_cast<ConstantExpr>(calledValue));
      if (!extracted) {
        /* TODO: unexpected value... */
        assert(false);
      }
      calledFunction = extracted;
    }
  }

  if (calledFunction == nullptr) {
    /* the called value should be a function pointer */
    if (usePA && aa) {
      resolveIndirectCallByPA(calledValue, targets);
    } else {
      Type *calledType = calledValue->getType();
      resolveIndirectCallByType(calledType, targets);
    }
  } else {
    targets.insert(calledFunction);
  }
}

void ReachabilityAnalysis::resolveIndirectCallByType(llvm::Type *calledType,
                                                     FunctionSet &targets) {
  llvm::Type *elementType =
      llvm::dyn_cast<llvm::PointerType>(calledType)->getPointerElementType();
  if (elementType == nullptr) {
    return;
  }

  llvm::FunctionType *functionType =
      llvm::dyn_cast<llvm::FunctionType>(elementType);
  if (functionType == nullptr) {
    return;
  }

  auto i = functionTypeMap.find(functionType);
  if (i != functionTypeMap.end()) {
    FunctionSet &functions = i->second;
    targets.insert(functions.begin(), functions.end());
  }
}

void ReachabilityAnalysis::resolveIndirectCallByPA(Value *calledValue,
                                                   FunctionSet &targets) {
  if (!calledValue) {
    return;
  }

  if (!aa->getPTA()->getPAG()->hasValueNode(SVFValueFrom(calledValue))) {
    debugs << "WARNING: no PAG node for: " << *calledValue << "\n";
    return;
  }

  SVF::NodeID id =
      aa->getPTA()->getPAG()->getValueNode(SVFValueFrom((calledValue)));
  const SVF::PointsTo &pts = aa->getPTA()->getPts(id);

  if (pts.empty()) {
    return;
  }

  for (SVF::PointsTo::iterator i = pts.begin(); i != pts.end(); ++i) {
    SVF::NodeID nodeId = *i;
    SVF::PAGNode *pagNode = aa->getPTA()->getPAG()->getGNode(nodeId);
    if (isa<SVF::ObjVar>(pagNode)) {
      SVF::ObjVar *obj = dyn_cast<SVF::ObjVar>(pagNode);
      const Value *value = LLVMValueFrom(obj->getMemObj()->getValue());
      if (isa<Function>(value)) {
        const Function *f = dyn_cast<const Function>(value);
        targets.insert((Function *)(f));
      }
    }
  }
}

Function *ReachabilityAnalysis::extractFunction(ConstantExpr *ce) {
  if (!ce->isCast()) {
    return NULL;
  }

  Value *value = ce->getOperand(0);
  if (isa<Function>(value)) {
    return dyn_cast<Function>(value);
  }

  if (isa<GlobalAlias>(value)) {
    Constant *aliasee = dyn_cast<GlobalAlias>(value)->getAliasee();
    if (isa<Function>(aliasee)) {
      return dyn_cast<Function>(aliasee);
    }
    if (isa<ConstantExpr>(aliasee)) {
      return extractFunction(dyn_cast<ConstantExpr>(aliasee));
    }
  }

  return NULL;
}

void ReachabilityAnalysis::updateCallMap(Instruction *callInst,
                                         FunctionSet &targets) {
  callMap[callInst].insert(targets.begin(), targets.end());
}

void ReachabilityAnalysis::updateRetMap(Instruction *callInst,
                                        FunctionSet &targets) {
  Instruction *retInst = callInst->getNextNode();
  for (FunctionSet::iterator i = targets.begin(); i != targets.end(); i++) {
    Function *f = *i;
    retMap[f].insert(retInst);
  }
}

ReachabilityAnalysis::FunctionSet &
ReachabilityAnalysis::getReachableFunctions(Function *f) {
  ReachabilityMap::iterator i = reachabilityMap.find(f);
  if (i == reachabilityMap.end()) {
    assert(false);
  }

  return i->second;
}

void ReachabilityAnalysis::getReachableInstructions(
    vector<CallInst *> &callSites, InstructionSet &result) {
  stack<Instruction *> stack;
  InstructionSet visited;

  for (vector<CallInst *>::iterator i = callSites.begin(); i != callSites.end();
       i++) {
    CallInst *callInst = *i;
    stack.push(callInst->getNextNode());
  }

  while (!stack.empty()) {
    /* fetch an instruction */
    Instruction *inst = stack.top();
    stack.pop();

    /* check if already visited */
    if (visited.find(inst) != visited.end()) {
      continue;
    }

    if (isa<CallInst>(inst)) {
      CallMap::iterator i = callMap.find(inst);
      if (i != callMap.end()) {
        FunctionSet &targets = i->second;
        for (FunctionSet::iterator j = targets.begin(); j != targets.end();
             j++) {
          Function *f = *j;
          if (f->isDeclaration()) {
            continue;
          }

          Instruction *first = &*f->begin()->begin();
          stack.push(first);
        }
      }

      stack.push(inst->getNextNode());

    } else if (isa<ReturnInst>(inst)) {
      Function *src = inst->getParent()->getParent();
      RetMap::iterator i = retMap.find(src);
      if (i != retMap.end()) {
        InstructionSet &targets = i->second;
        for (InstructionSet::iterator j = targets.begin(); j != targets.end();
             j++) {
          Instruction *retInst = *j;
          stack.push(retInst);
        }
      }

    } else if (inst->isTerminator()) {
      for (unsigned int i = 0; i < inst->getNumSuccessors(); i++) {
        BasicBlock *bb = inst->getSuccessor(i);
        stack.push(&*bb->begin());
      }
    } else {
      stack.push(inst->getNextNode());
    }

    visited.insert(inst);
    result.insert(inst);
  }
}

void ReachabilityAnalysis::getCallTargets(llvm::Instruction *inst,
                                          FunctionSet &result) {
  if (inst->getOpcode() != Instruction::Call) {
    return;
  }

  CallMap::iterator i = callMap.find(inst);
  if (i == callMap.end()) {
    return;
  }

  result = i->second;
}

void ReachabilityAnalysis::dumpReachableFunctions() {
  /* get all reachable functions */
  FunctionSet &reachable = getReachableFunctions(entryFunction);

  debugs << "### " << reachable.size() << " reachable functions ###\n";
  for (FunctionSet::iterator i = reachable.begin(); i != reachable.end(); i++) {
    Function *f = *i;
    debugs << "    " << f->getName() << "\n";
  }
  debugs << "\n";
}
