#ifndef SOLVERADAPTER_H
#define SOLVERADAPTER_H

#include "SolverTheory.h"

namespace klee {

class ExprHandle;

class SolverAdapter {
public:
  /* Arrays theory */
  struct Arrays : SolverTheory {
    Arrays();
    virtual ExprHandle read(const ExprHandle &array, const ExprHandle &index);
    virtual ExprHandle write(const ExprHandle &array, const ExprHandle &index,
                             const ExprHandle &value);
  };

  /*
   * Theory of Bit Vectors. Supports all operations with
   * bit words available in modern machines.
   */
  struct BV : SolverTheory {
    BV();
    virtual ExprHandle add(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle sub(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle mul(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle udiv(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle sdiv(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle shl(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle lshr(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle sext(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle uext(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle band(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle bor(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle bxor(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle bnot(const ExprHandle &lhs);

    virtual ExprHandle ult(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle ule(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle slt(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle sle(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle extract();
  };

  struct LIA : SolverTheory {
    LIA();
    virtual ExprHandle add(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle sub(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle mult(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle sext(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle uext(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle ult(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle ule(const ExprHandle &lhs, const ExprHandle &rhs);

    virtual ExprHandle slt(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle sle(const ExprHandle &lhs, const ExprHandle &rhs);
  };

  /* Propositional logic theory */
  struct Propositional {
    virtual ExprHandle land(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle lor(const ExprHandle &lhs, const ExprHandle &rhs);
    virtual ExprHandle lnot(const ExprHandle &lhs);
    virtual ExprHandle lite(const ExprHandle &cond, const ExprHandle &lhs,
                            const ExprHandle &rhs);
  };

  virtual ~SolverAdapter() = 0;
};

} // namespace klee

#endif
