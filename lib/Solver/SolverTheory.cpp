#include "SolverTheory.h"

#include "SolverAdapter.h"
#include "klee/ADT/Ref.h"

using namespace klee;

SolverTheory::SolverTheory(const ref<SolverAdapter> &solverAdapter)
    : solverAdapter(solverAdapter) {
  castMapping[ARRAYS] = &SolverTheory::castToArray;
  castMapping[BV] = &SolverTheory::castToBV;
  castMapping[BOOL] = &SolverTheory::castToBool;
  castMapping[FPBV] = &SolverTheory::castToFPBV;
  castMapping[LIA] = &SolverTheory::castToLIA;
}

ref<ExprHandle> SolverTheory::castToArray(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverTheory::castToBV(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverTheory::castToBool(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverTheory::castToFPBV(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverTheory::castToLIA(const ref<ExprHandle> &arg) {
  return nullptr;
}

/* Common bool operators */
ref<ExprHandle> SolverTheory::land(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  return solverAdapter->propAnd(lhs, rhs);
}

ref<ExprHandle> SolverTheory::lor(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  return solverAdapter->propOr(lhs, rhs);
}

ref<ExprHandle> SolverTheory::lxor(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  return solverAdapter->propXor(lhs, rhs);
}

ref<ExprHandle> SolverTheory::lnot(const ref<ExprHandle> &arg) {
  return solverAdapter->propNot(arg);
}

ref<ExprHandle> SolverTheory::lite(const ref<ExprHandle> &cond,
                                   const ref<ExprHandle> &onTrue,
                                   const ref<ExprHandle> &onFalse) {
  return solverAdapter->propIte(cond, onTrue, onFalse);
}

ref<ExprHandle> SolverTheory::eq(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  return solverAdapter->eq(lhs, rhs);
}

SolverTheory::Sort ExprHandle::sort() const { return parent->theorySort; }

ref<ExprHandle> ExprHandle::castTo(SolverTheory::Sort targetSort) {
  return parent->castTo(targetSort, ref<ExprHandle>(this));
}

void ExprHandle::pushSideConstraint(const ref<ExprHandle> &constraint) {
  sideConstraints.push_back(constraint);
}

ref<ExprHandle> SolverTheory::castTo(SolverTheory::Sort sort,
                                     const ref<ExprHandle> &arg) {
  if (castMapping.count(sort) == 0) {
    return nullptr;
  }
  const cast_function_t castFunction = castMapping.at(sort);
  return (*this.*castFunction)(arg);
}
