#include "SolverTheory.h"

#include "SolverAdapter.h"

#include "klee/ADT/Ref.h"
#include "llvm/Support/Casting.h"

using namespace klee;

ref<ExprHandle> FPBV::add(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) {
  return solverAdapter->bvFAdd(lhs, rhs);
}

ref<ExprHandle> FPBV::sub(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) {
  return solverAdapter->bvFSub(lhs, rhs);
}

ref<ExprHandle> FPBV::mul(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) {
  return solverAdapter->bvFMul(lhs, rhs);
}

ref<ExprHandle> FPBV::div(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) {
  return solverAdapter->bvFDiv(lhs, rhs);
}
