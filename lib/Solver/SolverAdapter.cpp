#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"

#include <optional>

using namespace klee;

ref<ExprHandle> SolverAdapter::bvSort(uint64_t width) { return nullptr; }

/* Floats section */
ref<ExprHandle> SolverAdapter::bvFConst(const llvm::APFloat &val) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFAdd(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFSub(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvFMul(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFDiv(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFRem(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFMax(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFMin(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvFOEq(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFOLt(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFOLe(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvFIsNaN(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFIsInfinite(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFIsNormal(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFIsSubnormal(const ref<ExprHandle> &arg) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvFSqrt(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFRint(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFAbs(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvFNeg(const ref<ExprHandle> &arg) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvConst(const llvm::APInt &val) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvAdd(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvSub(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvMul(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvUDiv(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvSDiv(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvURem(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvSRem(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvAnd(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvOr(const ref<ExprHandle> &lhs,
                                    const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvXor(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvNot(const ref<ExprHandle> &arg) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvUle(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvSle(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvUlt(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvSlt(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvZExt(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvSExt(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvShl(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvAShr(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvLShr(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::bvExtract(const ref<ExprHandle> &expr,
                                         const ref<ExprHandle> &lb,
                                         const ref<ExprHandle> &rb) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::bvConcat(const ref<ExprHandle> &lhs,
                                        const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::liaConst(const llvm::APInt &val) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::liaAdd(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::liaDub(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::liaMul(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::liaLe(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::liaLt(const ref<ExprHandle> &lhs,
                                     const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::array(const ref<ExprHandle> &domainSort,
                                     const ref<ExprHandle> &rangeSort) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::read(const ref<ExprHandle> &arr,
                                    const ref<ExprHandle> &idx) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::write(const ref<ExprHandle> &arr,
                                     const ref<ExprHandle> &idx,
                                     const ref<ExprHandle> &val) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::propConst(bool val) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::propAnd(const ref<ExprHandle> &lhs,
                                       const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::propOr(const ref<ExprHandle> &lhs,
                                      const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::propXor(const ref<ExprHandle> &lhs,
                                       const ref<ExprHandle> &rhs) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::propNot(const ref<ExprHandle> &arg) {
  return nullptr;
}
ref<ExprHandle> SolverAdapter::propIte(const ref<ExprHandle> &cond,
                                       const ref<ExprHandle> &onTrue,
                                       const ref<ExprHandle> &onFalse) {
  return nullptr;
}

ref<ExprHandle> SolverAdapter::eq(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  return nullptr;
}

SolverAdapter::~SolverAdapter() {}
