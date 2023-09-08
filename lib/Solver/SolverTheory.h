#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <unordered_map>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverAdapter.h"
#include "SolverBuilder.h"

#include <unordered_map>

namespace klee {

typedef std::vector<ref<ExprHandle>> ExprHandleList;

class SolverAdapter;

struct SolverTheory {
public:
  enum Sort { UNKNOWN, ARRAYS, BOOL, BV, FPBV, LIA };
  const Sort theorySort = Sort::UNKNOWN;

  typedef ref<ExprHandle> (SolverTheory::*cast_function_t)(
      const ref<ExprHandle> &);

  /* FIXME: figure out better style */
  std::unordered_map<SolverTheory::Sort, cast_function_t> castMapping;

  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  ref<SolverAdapter> solverAdapter;

protected:
  virtual ref<ExprHandle> castToArray(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> castToBV(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> castToBool(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> castToFPBV(const ref<ExprHandle> &arg) {
    return nullptr;
  }
  virtual ref<ExprHandle> castToLIA(const ref<ExprHandle> &arg) {
    return nullptr;
  }

public:
  SolverTheory() {
    castMapping[ARRAYS] = &castToArray;
    castMapping[BV] = &castToBV;
    castMapping[BOOL] = &castToBool;
    castMapping[FPBV] = &castToFPBV;
    castMapping[LIA] = &castToLIA;
  }

  virtual ref<ExprHandle> sort() = 0;
  virtual ref<ExprHandle> translate(const ref<Expr> &,
                                    const ExprHandleList &) = 0;

  /* Common bool operators */
  ref<ExprHandle> land(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->propAnd(lhs, rhs);
  }

  ref<ExprHandle> lor(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->propOr(lhs, rhs);
  }

  ref<ExprHandle> lxor(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->propXor(lhs, rhs);
  }

  ref<ExprHandle> lnot(const ref<ExprHandle> &arg) {
    return solverAdapter->propNot(arg);
  }

  ref<ExprHandle> lite(const ref<ExprHandle> &cond,
                       const ref<ExprHandle> &onTrue,
                       const ref<ExprHandle> &onFalse) {
    return solverAdapter->propIte(cond, onTrue, onFalse);
  }

  ref<ExprHandle> eq(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->eq(lhs, rhs);
  }

  ref<ExprHandle> castTo(Sort sort, const ref<ExprHandle> &arg) {
    if (castMapping.count(sort) == 0) {
      return nullptr;
    }
    const cast_function_t castFunction = castMapping.at(sort);
    return (*this.*castFunction)(arg);
  }

  virtual ~SolverTheory() = default;
};

struct LIA : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(const ref<Expr> &, const ExprHandleList &);

public:
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> mul(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> sext(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> zext(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> ult(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> ule(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> slt(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sle(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
};

struct FPBV : public SolverTheory {
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> mul(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> div(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> toInt(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> fromInt(const ref<ExprHandle> &arg);
};

} // namespace klee

#endif
