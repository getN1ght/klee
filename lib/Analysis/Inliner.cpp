#include <cassert>
#include <string>
#include <vector>

#include <llvm/Analysis/InlineCost.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "klee/Internal/Analysis/Inliner.h"
#include "klee/Internal/Analysis/ReachabilityAnalysis.h"

void Inliner::run() {
  if (functions.empty()) {
    return;
  }

  for (auto functionName : targets) {
    llvm::Function *entry = module->getFunction(functionName);
    assert(entry);

    /* we can't use pointer analysis at this point... */
    // TODO: why?....
    ReachabilityAnalysis::FunctionSet reachable;
    ra.computeReachableFunctions(entry, false, reachable);

    for (auto f : reachable) {
      if (f->isDeclaration()) {
        continue;
      }
      inlineCalls(f, functions);
    }
  }
}

void Inliner::inlineCalls(llvm::Function *f,
                          const std::vector<std::string> &functions) {
  std::vector<llvm::CallInst *> calls;

  for (llvm::inst_iterator i = inst_begin(f); i != inst_end(f); i++) {
    llvm::Instruction *inst = &*i;
    if (inst->getOpcode() != llvm::Instruction::Call) {
      continue;
    }

    llvm::CallInst *callInst = llvm::dyn_cast<llvm::CallInst>(inst);
    llvm::Function *calledFunction = callInst->getCalledFunction();
    if (!calledFunction) {
      /* TODO: handle aliases, ... */
      continue;
    }

    if (std::find(functions.begin(), functions.end(),
                  calledFunction->getName().str()) == functions.end()) {
      continue;
    }

    calls.push_back(callInst);
  }

  for (auto i : calls) {
    /* inline function call */
    llvm::InlineFunctionInfo ifi;
    [[maybe_unused]] llvm::InlineResult inlineResult = InlineFunction(*i, ifi);
    assert(inlineResult.isSuccess());
  }
}
