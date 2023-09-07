#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <unordered_map>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverBuilder.h"
#include "SolverAdapter.h"

namespace klee {

typedef std::vector<ref<ExprHandle>> ArgumentsList;

class SolverAdapter;

struct SolverTheory {
protected:
  ref<SolverAdapter> solverAdapter;

public:
  enum Sort { UNKNOWN, ARRAYS, BOOL, BV, FPBV, LIA };
  Sort theorySort = Sort::UNKNOWN;

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
  virtual ref<ExprHandle> sort() = 0;
  virtual ref<ExprHandle> translate(const ref<Expr> &, const ArgumentsList &) = 0;
  
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
    return nullptr;
    // return solverAdapter->e
  }

  ref<ExprHandle> castTo(Sort sort) {
    switch (sort) {
      case ARRAYS:
        return castToArray(nullptr);
      case BV:
        return castToBV(nullptr);
      case BOOL:
        return castToBool(nullptr);
      case FPBV:
        return castToFPBV(nullptr);
      case LIA:
        return castToLIA(nullptr);
    }
  }

  virtual ~SolverTheory() = default;
};


struct LIA : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(const ref<Expr> &, const ArgumentsList &);

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