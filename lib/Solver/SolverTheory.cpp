#include "SolverTheory.h"

#include "SolverAdapter.h"
#include "klee/ADT/Ref.h"

#include "klee/Expr/ExprHashMap.h"

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

ref<ExprHandle> SolverTheory::eq(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  return solverAdapter->eq(lhs, rhs);
}

SolverTheory::Sort ExprHandle::sort() const { return parent->theorySort; }

ref<ExprHandle> ExprHandle::castTo(SolverTheory::Sort targetSort) {
  return parent->castTo(targetSort, ref<ExprHandle>(this));
}

ref<ExprHandle> SolverTheory::castTo(SolverTheory::Sort sort,
                                     const ref<ExprHandle> &arg) {
  if (castMapping.count(sort) == 0) {
    return nullptr;
  }
  const cast_function_t castFunction = castMapping.at(sort);
  return (this->*castFunction)(arg);
}

ref<ExprHandle> CompleteResponse::expr() const {
  return handle;
}

CompleteResponse
IncompleteResponse::complete(const ExprHashMap<ref<ExprHandle>> &required) {
  // TODO: accept a PROVIDER, not just just a MAP.
  for (const ref<Expr> &expr : toBuild) {
    if (required.count(expr) == 0) {
      llvm::errs() << "Incomplete response error\n";
      std::abort();
    }
  }

  return CompleteResponse(completer(required));
}
