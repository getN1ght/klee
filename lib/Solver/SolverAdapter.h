#ifndef SOLVERADAPTER_H
#define SOLVERADAPTER_H

#include "klee/ADT/Ref.h"
#include <cstdint>

namespace llvm {
class APInt;
class APFloat;
} // namespace llvm

namespace klee {

class SolverHandle {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

public:
  virtual ~SolverHandle() = default;
};

class SortHandle {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

public:
  virtual ~SortHandle() = default;
};

class SolverAdapter {
public:
  virtual ref<SortHandle> bvSort(uint64_t width);

  /* Floats section */
  virtual ref<SolverHandle> bvFConst(const llvm::APFloat &val);
  virtual ref<SolverHandle> bvFAdd(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFSub(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvFMul(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFDiv(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFRem(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFMax(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFMin(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvFOEq(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFOLt(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvFOLe(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvFIsNaN(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> bvFIsInfinite(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> bvFIsNormal(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> bvFIsSubnormal(const ref<SolverHandle> &arg);

  virtual ref<SolverHandle> bvFSqrt(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> bvFRint(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> bvFAbs(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> bvFNeg(const ref<SolverHandle> &arg);

  /* Arithmetics section */
public:
  virtual ref<SolverHandle> bvConst(const llvm::APInt &val);
  virtual ref<SolverHandle> bvAdd(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvSub(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvMul(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvUDiv(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvSDiv(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvURem(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvSRem(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvAnd(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvOr(const ref<SolverHandle> &lhs,
                                 const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvXor(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvNot(const ref<SolverHandle> &arg);

  virtual ref<SolverHandle> bvUle(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvSle(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvUlt(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvSlt(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvZExt(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvSExt(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvShl(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvAShr(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> bvLShr(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> bvExtract(const ref<SolverHandle> &expr,
                                      const ref<SolverHandle> &lb,
                                      const ref<SolverHandle> &rb);
  virtual ref<SolverHandle> bvConcat(const ref<SolverHandle> &lhs,
                                     const ref<SolverHandle> &rhs);
  /* LIA section */
public:
  virtual ref<SortHandle> liaSort();

  virtual ref<SolverHandle> liaConst(const llvm::APInt &val);
  virtual ref<SolverHandle> liaAdd(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> liaDub(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> liaMul(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);

  virtual ref<SolverHandle> liaLe(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> liaLt(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs);

  /* Arrays section */
public:
  virtual ref<SortHandle> arraySort(const ref<SortHandle> &domainSort,
                                    const ref<SortHandle> &rangeSort);

  virtual ref<SolverHandle> array(const std::string &name,
                                  const ref<SortHandle> &sort);

  virtual ref<SolverHandle> read(const ref<SolverHandle> &arr,
                                 const ref<SolverHandle> &idx);
  virtual ref<SolverHandle> write(const ref<SolverHandle> &arr,
                                  const ref<SolverHandle> &idx,
                                  const ref<SolverHandle> &val);

  /* Propositional section */
public:
  virtual ref<SortHandle> boolSort();

  virtual ref<SolverHandle> propConst(bool val);
  virtual ref<SolverHandle> propAnd(const ref<SolverHandle> &lhs,
                                    const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> propOr(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> propXor(const ref<SolverHandle> &lhs,
                                    const ref<SolverHandle> &rhs);
  virtual ref<SolverHandle> propNot(const ref<SolverHandle> &arg);
  virtual ref<SolverHandle> propIte(const ref<SolverHandle> &cond,
                                    const ref<SolverHandle> &onTrue,
                                    const ref<SolverHandle> &onFalse);

  virtual ref<SolverHandle> eq(const ref<SolverHandle> &lhs,
                               const ref<SolverHandle> &rhs);

public:
  virtual ~SolverAdapter() = 0;
};

} // namespace klee

#endif
