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

ref<TheoryHandle> SolverTheory::castToArray(const ref<TheoryHandle> &arg) {
  return nullptr;
}
ref<TheoryHandle> SolverTheory::castToBV(const ref<TheoryHandle> &arg) {
  return nullptr;
}
ref<TheoryHandle> SolverTheory::castToBool(const ref<TheoryHandle> &arg) {
  return nullptr;
}
ref<TheoryHandle> SolverTheory::castToFPBV(const ref<TheoryHandle> &arg) {
  return nullptr;
}
ref<TheoryHandle> SolverTheory::castToLIA(const ref<TheoryHandle> &arg) {
  return nullptr;
}

ref<ExprHandle> SolverTheory::eq(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  return solverAdapter->eq(lhs, rhs);
}

ref<ExprHandle> SolverTheory::castTo(SolverTheory::Sort sort,
                                     const ref<ExprHandle> &arg) {
  if (castMapping.count(sort) == 0) {
    return nullptr;
  }
  const cast_function_t castFunction = castMapping.at(sort);
  return (this->*castFunction)(arg);
}

ref<ExprHandle> CompleteTheoryHandle::expr() const {
  return handle;
}

CompleteTheoryHandle
IncompleteResponse::complete(const ExprHashMap<ref<ExprHandle>> &required) {
  // TODO: accept a PROVIDER, not just just a MAP.
  for (const ref<Expr> &expr : toBuild) {
    if (required.count(expr) == 0) {
      llvm::errs() << "Incomplete response error\n";
      std::abort();
    }
  }

  return CompleteTheoryHandle(parent, completer(required));
}
