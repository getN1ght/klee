#ifndef SOLVERADAPTER_H
#define SOLVERADAPTER_H

#include "SolverTheory.h"

namespace klee {

template <typename T> class ref;
class ExprHandle;

class SolverAdapter {
public:
  /* Arrays theory */
  struct Arrays : SolverTheory {
  protected:
    virtual ref<ExprHandle> applyHandler(Expr::Kind, const ArgumentsList &) = 0;

  public:
    virtual ref<ExprHandle> read(const ref<ExprHandle> &array,
                                 const ref<ExprHandle> &index);
    virtual ref<ExprHandle> write(const ref<ExprHandle> &array,
                                  const ref<ExprHandle> &index,
                                  const ref<ExprHandle> &value);
  };

  /*
   * Theory of Bit Vectors. Supports all operations with
   * bit words available in modern machines.
   */
  struct BV : SolverTheory {
  protected:
    virtual ref<ExprHandle> applyHandler(Expr::Kind, const ArgumentsList &) = 0;

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

    virtual ref<ExprHandle> extract();
  };

  struct LIA : SolverTheory {
  protected:
    virtual ref<ExprHandle> applyHandler(Expr::Kind, const ArgumentsList &) = 0;

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
  struct Propositional : SolverTheory {
  protected:
    virtual ref<ExprHandle> applyHandler(Expr::Kind, const ArgumentsList &) = 0;

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

  virtual ~SolverAdapter() = 0;
};

} // namespace klee

#endif
