#include <MemoryModel/PointerAnalysisImpl.h>
#include <SVF-LLVM/LLVMModule.h>
#include <SVFIR/SVFVariables.h>
#include <Util/GeneralType.h>
#include <dg/PointerAnalysis/Pointer.h>
#include <stdio.h>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include "dg/Offset.h"
#include "dg/PointerAnalysis/PSNode.h"
#include "dg/PointerAnalysis/PointerGraph.h"

#include "klee/Internal/Analysis/AAPass.h"
#include "klee/Internal/Analysis/SVFPointerAnalysis.h"
#include "klee/Internal/Analysis/Util.h"

using namespace llvm;
using namespace dg::pta;

void SVFPointerAnalysis::run() {
  /* update virtual call related nodes */
  handleVirtualCalls();

  for (auto &v : pta->getBuilder()->getNodesMap()) {
    PSNode *node = *std::prev(v.second.end());
    handleNode(node);
  }
}

void SVFPointerAnalysis::handleNode(PSNode *node) {
  switch (node->getType()) {
  case PSNodeType::LOAD:
    handleLoad(node);
    break;

  case PSNodeType::STORE:
    handleStore(node);
    break;

  case PSNodeType::GEP:
    handleGep(node);
    break;

  case PSNodeType::CAST:
    handleCast(node);
    break;

  case PSNodeType::CONSTANT:
    break;

  case PSNodeType::CALL_RETURN:
  case PSNodeType::RETURN:
  case PSNodeType::PHI:
    handlePhi(node);
    break;

  case PSNodeType::CALL_FUNCPTR:
    break;

  case PSNodeType::MEMCPY:
    /* TODO: handle? */
    break;

  case PSNodeType::ALLOC:
  case PSNodeType::FUNCTION:
    // assert(node->doesPointsTo(node, 0));
    // assert(node->pointsTo.size() == 1);
  case PSNodeType::CALL:
  case PSNodeType::ENTRY:
  case PSNodeType::NOOP:
    break;

  default:
    assert(false);
  }
}

void SVFPointerAnalysis::handleVirtualCalls() {
  std::set<PSNode *> visited;
  bool changed = true;

  while (changed) {
    std::set<PSNode *> discovered;
    changed = false;

    /* first, get all relevant nodes */
    for (auto &v : pta->getBuilder()->getNodesMap()) {
      PSNode *node = *v.second.end();
      if (node->getType() != PSNodeType::CALL_FUNCPTR) {
        continue;
      }

      if (visited.find(node) == visited.end()) {
        discovered.insert(node);
        changed = true;
      }
    }

    for (PSNode *node : discovered) {
      handleFuncPtr(node);
      visited.insert(node);
    }
  }
}

void SVFPointerAnalysis::handleLoad(PSNode *node) {
  handleOperand(node);
  handleOperand(node->getOperand(0));
}

void SVFPointerAnalysis::handleStore(PSNode *node) {
  handleOperand(node);
  handleOperand(node->getOperand(1));
}
void SVFPointerAnalysis::handleGep(PSNode *node) {
  handleOperand(node);
  handleOperand(node->getOperand(0));
}

void SVFPointerAnalysis::handleCast(PSNode *node) {
  handleOperand(node);
  handleOperand(node->getOperand(0));
}

void SVFPointerAnalysis::handlePhi(PSNode *node) {
  /* TODO: check if required! */
  handleOperand(node);
  for (PSNode *op : node->getOperands()) {
    handleOperand(op);
  }
}

void SVFPointerAnalysis::handleFuncPtr(PSNode *node) {
  PSNode *operand = node->getOperand(0);
  handleOperand(operand);

  /* now, operand->pointsTo is updated */
  for (const Pointer &ptr : operand->pointsTo) {
    if (ptr.isValid()) {
      functionPointerCall(node, ptr.target);
    }
  }
}

/* based on the code from DG */
bool SVFPointerAnalysis::functionPointerCall(PSNode *callsite, PSNode *called) {
  if (!isa<Function>(called->getUserData<Value>())) {
    return false;
  }

  const Function *f = called->getUserData<Function>();
  const CallInst *callInst = callsite->getUserData<CallInst>();

  /* TODO: make a partial compatability check */
  // if (!llvmutils::callIsCompatible(F, CI))
  //     return false;

  if (f->size() == 0) {
    return callsite->getPairedNode()->addPointsTo(dg::pta::UnknownPointer);
  }

  auto seq = pta->getBuilder()->createFuncptrCall(callInst, f);
  assert(seq.getFirst() && seq.getLast());

  PSNode *paired = callsite->getPairedNode();
  paired->addOperand(seq.getLast());

  if (callsite->successorsNum() == 1 &&
      callsite->getSingleSuccessor() == paired) {
    callsite->replaceSingleSuccessor(seq.getFirst());
  } else {
    callsite->addSuccessor(seq.getFirst());
  }

  seq.getLast()->addSuccessor(paired);

  return true;
}

void SVFPointerAnalysis::handleOperand(PSNode *operand) {
  Value *value = operand->getUserData<Value>();
  if (!value) {
    return;
  }

  if (!aa->getPTA()->getPAG()->hasValueNode(SVFValueFrom(value))) {
    /* TODO: not a pointer? */
    return;
  }

  SVF::NodeID id = aa->getPTA()->getPAG()->getValueNode(SVFValueFrom(value));
  const SVF::PointsTo &pts = aa->getPTA()->getPts(id);

  if (pts.empty()) {
    operand->addPointsTo(NULLPTR, 0);
    return;
  }

  for (SVF::PointsTo::iterator i = pts.begin(); i != pts.end(); ++i) {
    SVF::NodeID node_id = *i;
    SVF::PAGNode *pagnode = aa->getPTA()->getPAG()->getGNode(node_id);
    if (isa<SVF::ObjVar>(pagnode)) {
      updatePointsTo(operand, pagnode);
    }
  }
}

void SVFPointerAnalysis::updatePointsTo(PSNode *operand,
                                        SVF::PAGNode *pagnode) {
  int kind = pagnode->getNodeKind();
  SVF::ObjVar *obj_node = NULL;
  SVF::GepObjVar *gepobj_node = NULL;
  PSNode *alloc_node = NULL;
  uint64_t offset = 0;

  switch (kind) {
  case SVF::PAGNode::ObjNode:
  case SVF::PAGNode::FIObjNode:
    /* TODO: handle FIObjNode */
    obj_node = dyn_cast<SVF::ObjVar>(pagnode);
    alloc_node = getAllocNode(obj_node);
    offset = 0;
    break;

  case SVF::PAGNode::GepObjNode:
    gepobj_node = dyn_cast<SVF::GepObjVar>(pagnode);
    alloc_node = getAllocNode(gepobj_node);
    offset = getAllocNodeOffset(gepobj_node);
    break;

  case SVF::PAGNode::DummyObjNode:
    /* TODO: are we supposed to do something? */
    return;

  default:
    assert(false);
    return;
  }

  if (!alloc_node) {
    return;
  }

  /* add to PointsTo set */
  operand->addPointsTo(Pointer(alloc_node, offset));
}

PSNode *SVFPointerAnalysis::getAllocNode(SVF::ObjVar *node) {
  /* get SVF memory object (allocation site) */
  const SVF::MemObj *mo = node->getMemObj();

  /* get corresponding DG node */
  PSNode *ref_node = pta->getPointsToNode(LLVMValueFrom(mo->getValue()));
  if (!ref_node) {
    /* TODO: check why DG does not have this allocation site */
    // assert(false);
  }

  return ref_node;
}

uint64_t SVFPointerAnalysis::getAllocNodeOffset(SVF::GepObjVar *node) {
  SVF::APOffset offsetInBytes =
      node->getConstantFieldIdx() * node->getType()->getByteSize();

  const SVF::MemObj *mo = node->getMemObj();
  if (mo->isArray()) {
    /* arrays are handled insensitively */
    return dg::Offset::getUnknown().offset;
  }

  return offsetInBytes;
}
