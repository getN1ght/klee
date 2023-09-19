#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"

#include <optional>

using namespace klee;

ref<SortHandle> SolverAdapter::bvSort(uint64_t width) { return nullptr; }

/* Floats section */
ref<SolverHandle> SolverAdapter::bvFConst(const llvm::APFloat &val) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFAdd(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFSub(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvFMul(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFDiv(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFRem(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFMax(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFMin(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvFOEq(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFOLt(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFOLe(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvFIsNaN(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFIsInfinite(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFIsNormal(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFIsSubnormal(const ref<SolverHandle> &arg) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvFSqrt(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFRint(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFAbs(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvFNeg(const ref<SolverHandle> &arg) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvConst(const llvm::APInt &val) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvAdd(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvSub(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvMul(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvUDiv(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvSDiv(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvURem(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvSRem(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvAnd(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvOr(const ref<SolverHandle> &lhs,
                                      const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvXor(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvNot(const ref<SolverHandle> &arg) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvUle(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvSle(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvUlt(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvSlt(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvZExt(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvSExt(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvShl(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvAShr(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvLShr(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::bvExtract(const ref<SolverHandle> &expr,
                                           const ref<SolverHandle> &lb,
                                           const ref<SolverHandle> &rb) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::bvConcat(const ref<SolverHandle> &lhs,
                                          const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SortHandle> SolverAdapter::liaSort() { return nullptr; }
ref<SolverHandle> SolverAdapter::liaConst(const llvm::APInt &val) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::liaAdd(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::liaDub(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::liaMul(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::liaLe(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::liaLt(const ref<SolverHandle> &lhs,
                                       const ref<SolverHandle> &rhs) {
  return nullptr;
}

ref<SortHandle> SolverAdapter::arraySort(const ref<SortHandle> &domainSort,
                                         const ref<SortHandle> &rangeSort) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::array(const std::string &name,
                                       const ref<SortHandle> &sort) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::read(const ref<SolverHandle> &arr,
                                      const ref<SolverHandle> &idx) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::write(const ref<SolverHandle> &arr,
                                       const ref<SolverHandle> &idx,
                                       const ref<SolverHandle> &val) {
  return nullptr;
}

ref<SortHandle> SolverAdapter::boolSort() { return nullptr; }

ref<SolverHandle> SolverAdapter::propConst(bool val) { return nullptr; }

ref<SolverHandle> SolverAdapter::propAnd(const ref<SolverHandle> &lhs,
                                         const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::propOr(const ref<SolverHandle> &lhs,
                                        const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::propXor(const ref<SolverHandle> &lhs,
                                         const ref<SolverHandle> &rhs) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::propNot(const ref<SolverHandle> &arg) {
  return nullptr;
}
ref<SolverHandle> SolverAdapter::propIte(const ref<SolverHandle> &cond,
                                         const ref<SolverHandle> &onTrue,
                                         const ref<SolverHandle> &onFalse) {
  return nullptr;
}

ref<SolverHandle> SolverAdapter::eq(const ref<SolverHandle> &lhs,
                                    const ref<SolverHandle> &rhs) {
  return nullptr;
}

SolverAdapter::~SolverAdapter() {}
