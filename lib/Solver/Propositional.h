#ifndef PROPOSITIONAL_H
#define PROPOSITIONAL_H

#include "SolverTheory.h"

#include "SolverAdapter.h"

#include "klee/ADT/Ref.h"
#include "llvm/Support/Casting.h"

namespace klee {

struct Propositional : public SolverTheory {
public:
  Propositional(const ref<SolverAdapter> &adapter) : SolverTheory(adapter) {}

  ref<TheoryResponse> translate(const ref<Expr> &expr,
                                const ExprHandleList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Constant: {
      return apply(&Propositional::constant, expr);
    }
    case Expr::Kind::And: {
      return apply(&Propositional::land, args[0], args[1]);
    }
    case Expr::Kind::Or: {
      return apply(&Propositional::lor, args[0], args[1]);
    }
    case Expr::Kind::Xor: {
      return apply(&Propositional::lxor, args[0], args[1]);
    }
    case Expr::Kind::Not: {
      return apply(&Propositional::lnot, args[0]);
    }
    case Expr::Kind::Select: {
      return apply(&Propositional::lite, args[0], args[1], args[2]);
    }
    default: {
      return nullptr;
    }
    }
  }

  ref<ExprHandle> constant(const ref<Expr> &expr) {
    ref<ConstantExpr> ce = cast<ConstantExpr>(expr);
    if (ce->getWidth() != 1) {
      return nullptr;
    }
    return solverAdapter->propConst(ce->isTrue());
  }

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
};

} // namespace klee
#endif
