#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <optional>
#include <unordered_map>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverBuilder.h"

namespace klee {

typedef std::vector<ref<ExprHandle>> ArgumentsList;

class SolverAdapter;

struct SolverTheory {
protected:
  ref<SolverAdapter> solverAdapter;

public:
  virtual ref<ExprHandle> translate(Expr::Kind, const ArgumentsList &) = 0;
  virtual ~SolverTheory() = default;
};

/* Arrays theory */
struct Arrays : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(Expr::Kind, const ArgumentsList &);

public:
  virtual ref<ExprHandle> read(const ref<ExprHandle> &array,
                               const ref<ExprHandle> &index) {
    return nullptr;
  }

  virtual ref<ExprHandle> write(const ref<ExprHandle> &array,
                                const ref<ExprHandle> &index,
                                const ref<ExprHandle> &value) {
    return nullptr;
  }
};

/*
 * Theory of Bit Vectors. Supports all operations with
 * bit words available in modern machines.
 */

struct BV : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(Expr::Kind, const ArgumentsList &);

public:
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> mul(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> udiv(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sdiv(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> shl(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> ashr(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> lshr(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> sext(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> zext(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> band(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bor(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bxor(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> bnot(const ref<ExprHandle> &lhs);

  virtual ref<ExprHandle> ult(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> ule(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> slt(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sle(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> extract(const ref<ExprHandle> &expr,
                                  const ref<ExprHandle> &lb,
                                  const ref<ExprHandle> &rb);
};

struct LIA : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(Expr::Kind, const ArgumentsList &);

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

/* Propositional logic theory */
struct Propositional : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(Expr::Kind, const ArgumentsList &);

public:
  virtual ref<ExprHandle> land(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> lor(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> lxor(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> lnot(const ref<ExprHandle> &lhs);
  virtual ref<ExprHandle> lite(const ref<ExprHandle> &cond,
                               const ref<ExprHandle> &lhs,
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