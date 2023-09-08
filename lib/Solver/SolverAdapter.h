#ifndef SOLVERADAPTER_H
#define SOLVERADAPTER_H

#include "SolverTheory.h"

#include "llvm/ADT/APInt.h"

namespace klee {

template <typename T> class ref;
class ExprHandle;

class SolverAdapter {
public:
  virtual ref<ExprHandle> bv(uint64_t width) {
    return nullptr;
  }

  /* Floats section */
  virtual ref<ExprHandle> bvFAdd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFSub(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvFMul(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFDiv(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFRem(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFMax(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFMin(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvFOEq(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFOLt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFOLe(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvFIsNaN(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFIsInfinite(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFIsNormal(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFIsSubnormal(const ref<ExprHandle> &arg) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvFSqrt(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFRint(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvFAbs(const ref<ExprHandle> &arg) { return nullptr; }
  virtual ref<ExprHandle> FNeg(const ref<ExprHandle> &arg) { return nullptr; }

  /* Arithmetics section */
public:
  virtual ref<ExprHandle> bvConst(const llvm::APInt &val) { return nullptr; }
  virtual ref<ExprHandle> bvAdd(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvSub(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvMul(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvUDiv(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvSDiv(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvURem(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvSRem(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvAnd(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvOr(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvXor(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvNot(const ref<ExprHandle> &arg) { return nullptr; }

  virtual ref<ExprHandle> bvUle(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvSle(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvUlt(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvSlt(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvZExt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvSExt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvShl(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvAShr(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvLShr(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> bvExtract(const ref<ExprHandle> &expr,
                                    const ref<ExprHandle> &lb,
                                    const ref<ExprHandle> &rb) {
    return nullptr;
  }
  virtual ref<ExprHandle> bvConcat(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  /* LIA section */
public:
  virtual ref<ExprHandle> liaAdd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> liaDub(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> liaMul(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ref<ExprHandle> liaLe(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> liaLt(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  /* Arrays section */
public:
  virtual ref<ExprHandle> array(const ref<ExprHandle> &domainSort,
                                const ref<ExprHandle> &rangeSort);

  virtual ref<ExprHandle> read(const ref<ExprHandle> &arr,
                               const ref<ExprHandle> &idx) {
    return nullptr;
  }
  virtual ref<ExprHandle> write(const ref<ExprHandle> &arr,
                                const ref<ExprHandle> &idx,
                                const ref<ExprHandle> &val) {
    return nullptr;
  }

  /* Propositional section */
public:
  virtual ref<ExprHandle> propAnd(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> propOr(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> propXor(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
    return nullptr;
  }
  virtual ref<ExprHandle> propNot(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> propIte(const ref<ExprHandle> &cond,
                                  const ref<ExprHandle> &onTrue,
                                  const ref<ExprHandle> &onFalse) {
    return nullptr;
  }

  virtual ref<ExprHandle> eq(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return nullptr;
  }

  virtual ~SolverAdapter() = 0;
};

} // namespace klee

#endif
