#ifndef SOLVERADAPTER_H
#define SOLVERADAPTER_H

#include "klee/ADT/Ref.h"
#include <cstdint>

namespace klee {

class ExprHandle;

class SolverAdapter {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

public:
  virtual ref<ExprHandle> bv(uint64_t width);

  /* Floats section */
  virtual ref<ExprHandle> bvFAdd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFSub(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvFMul(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFDiv(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFRem(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFMax(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFMin(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvFOEq(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFOLt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvFOLe(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvFIsNaN(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> bvFIsInfinite(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> bvFIsNormal(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> bvFIsSubnormal(const ref<ExprHandle> &arg);

  virtual ref<ExprHandle> bvFSqrt(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> bvFRint(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> bvFAbs(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> FNeg(const ref<ExprHandle> &arg);

  /* Arithmetics section */
public:
  virtual ref<ExprHandle> bvConst(const llvm::APInt &val);
  virtual ref<ExprHandle> bvAdd(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvSub(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvMul(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvUDiv(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvSDiv(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvURem(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvSRem(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvAnd(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvOr(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvXor(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvNot(const ref<ExprHandle> &arg);

  virtual ref<ExprHandle> bvUle(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvSle(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvUlt(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvSlt(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvZExt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvSExt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvShl(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvAShr(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bvLShr(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> bvExtract(const ref<ExprHandle> &expr,
                                    const ref<ExprHandle> &lb,
                                    const ref<ExprHandle> &rb);
  virtual ref<ExprHandle> bvConcat(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs);
  /* LIA section */
public:
  virtual ref<ExprHandle> liaAdd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> liaDub(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> liaMul(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> liaLe(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> liaLt(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs);

  /* Arrays section */
public:
  virtual ref<ExprHandle> array(const ref<ExprHandle> &domainSort,
                                const ref<ExprHandle> &rangeSort);

  virtual ref<ExprHandle> read(const ref<ExprHandle> &arr,
                               const ref<ExprHandle> &idx);
  virtual ref<ExprHandle> write(const ref<ExprHandle> &arr,
                                const ref<ExprHandle> &idx,
                                const ref<ExprHandle> &val);

  /* Propositional section */
public:
  virtual ref<ExprHandle> propAnd(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> propOr(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> propXor(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> propNot(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> propIte(const ref<ExprHandle> &cond,
                                  const ref<ExprHandle> &onTrue,
                                  const ref<ExprHandle> &onFalse);

  virtual ref<ExprHandle> eq(const ref<ExprHandle> &lhs,
                             const ref<ExprHandle> &rhs);

  virtual ~SolverAdapter() = 0;
};

} // namespace klee

#endif
