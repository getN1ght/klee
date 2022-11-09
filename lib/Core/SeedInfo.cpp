//===-- SeedInfo.cpp ------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SeedInfo.h"

#include "ExecutionState.h"
#include "Memory.h"
#include "TimingSolver.h"

#include "klee/ADT/KTest.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprUtil.h"
#include "klee/ADT/KTest.h"
#include "klee/Support/ErrorHandling.h"

using namespace klee;

KTestObject *SeedInfo::getNextInput(const MemoryObject *mo,
                                   bool byName) {
  if (byName) {
    unsigned i;
    
    for (i=0; i<input->numObjects; ++i) {
      KTestObject *obj = &input->objects[i];
      if (std::string(obj->name) == mo->name)
        if (used.insert(obj).second)
          return obj;
    }
    
    // If first unused input matches in size then accept that as
    // well.
    for (i=0; i<input->numObjects; ++i)
      if (!used.count(&input->objects[i]))
        break;
    if (i<input->numObjects) {
      KTestObject *obj = &input->objects[i];
      if (obj->numBytes == mo->size) {
        used.insert(obj);
        klee_warning_once(mo, "using seed input %s[%d] for: %s (no name match)",
                          obj->name, obj->numBytes, mo->name.c_str());
        return obj;
      }
    }
    
    klee_warning_once(mo, "no seed input for: %s", mo->name.c_str());
    return 0;
  } else {
    if (inputPosition >= input->numObjects) {
      return 0; 
    } else {
      return &input->objects[inputPosition++];
    }
  }
}

void SeedInfo::patchSeed(ExecutionState &state, 
                         ref<Expr> condition,
                         TimingSolver *solver) {
  ConstraintSet required(state.evaluateConstraintsWithSymcretes());
  ConstraintManager cm(required);
  cm.addConstraint(condition);

  // Try and patch direct reads first, this is likely to resolve the
  // problem quickly and avoids long traversal of all seed
  // values. There are other smart ways to do this, the nicest is if
  // we got a minimal counterexample from STP, in which case we would
  // just inject those values back into the seed.
  std::set< std::pair<const Array*, unsigned> > directReads;
  std::vector< ref<ReadExpr> > reads;
  findReads(condition, false, reads);
  for (std::vector< ref<ReadExpr> >::iterator it = reads.begin(), 
         ie = reads.end(); it != ie; ++it) {
    ReadExpr *re = it->get();
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(re->index)) {
      directReads.insert(std::make_pair(re->updates.root, 
                                        (unsigned) CE->getZExtValue(32)));
    }
  }
  
  for (std::set< std::pair<const Array*, unsigned> >::iterator
         it = directReads.begin(), ie = directReads.end(); it != ie; ++it) {
    const Array *array = it->first;
    unsigned i = it->second;
    ref<Expr> read = ReadExpr::create(UpdateList(array, 0),
                                      ConstantExpr::alloc(i, Expr::Int32));
    
    Assignment assignmentHack;

    // If not in bindings then this can't be a violation?
    Assignment::bindings_ty::iterator it2 = assignment.bindings.find(array);
    if (it2 != assignment.bindings.end()) {
      ref<Expr> isSeed = EqExpr::create(read, 
                                        ConstantExpr::alloc(it2->second[i], 
                                                            Expr::Int8));
      bool res;
      bool success =
          solver->mustBeFalse(state, required, isSeed, res, state.queryMetaData, assignmentHack);
      assert(success && "FIXME: Unhandled solver failure");
      (void) success;
      if (res) {
        ref<ConstantExpr> value;
        bool success =
            solver->getValue(required, read, value, state.queryMetaData);
        assert(success && "FIXME: Unhandled solver failure");            
        (void) success;
        it2->second[i] = value->getZExtValue(8);
        cm.addConstraint(EqExpr::create(
            read, ConstantExpr::alloc(it2->second[i], Expr::Int8)));
      } else {
        cm.addConstraint(isSeed);
      }
    }
  }

  bool res;

  Assignment symcreteExample;
  bool success = solver->mayBeTrue(
      state, state.evaluateConstraintsWithSymcretes(),
      assignment.evaluate(condition), res, state.queryMetaData, symcreteExample, true);
  assert(success && "FIXME: Unhandled solver failure");
  (void) success;
  if (res) {
    state.updateSymcretes(symcreteExample);

    ValidityCore checkExampleCore;
    bool stateHasNonAppropriateConstraints;
    assert(solver->getValidityCore(
               state.evaluateConstraintsWithSymcretes(),
               ConstantExpr::create(0, Expr::Bool), checkExampleCore,
               stateHasNonAppropriateConstraints, state.queryMetaData) &&
           "FIXME: Unhandled solver failure");

    if (!stateHasNonAppropriateConstraints) {
      return;
    }

  }
  
  // We could still do a lot better than this, for example by looking at
  // independence. But really, this shouldn't be happening often.
  for (Assignment::bindings_ty::iterator it = assignment.bindings.begin(), 
         ie = assignment.bindings.end(); it != ie; ++it) {
    const Array *array = it->first;
    ref<ConstantExpr> size =
        dyn_cast<ConstantExpr>(state.evaluateWithSymcretes(array->getSize()));
    assert(size && "Array has non-constant size");
    Assignment assignmentHack;
    for (unsigned i=0; i<size->getZExtValue(); ++i) {
      ref<Expr> read = ReadExpr::create(UpdateList(array, 0),
                                        ConstantExpr::alloc(i, Expr::Int32));
      ref<Expr> isSeed = EqExpr::create(read, 
                                        ConstantExpr::alloc(it->second[i], 
                                                            Expr::Int8));
      bool res;
      bool success =
          solver->mustBeFalse(state, required, isSeed, res, state.queryMetaData, assignmentHack);
      assert(success && "FIXME: Unhandled solver failure");
      (void) success;
      if (res) {
        ref<ConstantExpr> value;
        bool success =
            solver->getValue(required, read, value, state.queryMetaData);
        assert(success && "FIXME: Unhandled solver failure");            
        (void) success;
        it->second[i] = value->getZExtValue(8);
        cm.addConstraint(EqExpr::create(
            read, ConstantExpr::alloc(it->second[i], Expr::Int8)));
      } else {
        cm.addConstraint(isSeed);
      }
    }
  }

#ifndef NDEBUG
  {
    bool res;
    Assignment symcreteExample;
    bool success =
        solver->mayBeTrue(state, state.evaluateConstraintsWithSymcretes(),
                          assignment.evaluate(condition), res,
                          state.queryMetaData, symcreteExample, true);
    assert(success && "FIXME: Unhandled solver failure");            
    (void) success;
    assert(res && "seed patching failed");
    state.updateSymcretes(symcreteExample);
    
    ValidityCore checkExampleCore;
    bool stateHasNonAppropriateConstraints;
    assert(solver->getValidityCore(
               state.evaluateConstraintsWithSymcretes(),
               ConstantExpr::create(0, Expr::Bool), checkExampleCore,
               stateHasNonAppropriateConstraints, state.queryMetaData) &&
           "FIXME: Unhandled solver failure");

    assert(!stateHasNonAppropriateConstraints && "seed patching failed");
  }
#endif
}
