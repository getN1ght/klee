#ifndef PROPOSITIONAL_H
#define PROPOSITIONAL_H

#include "SolverTheory.h"

#include "SolverAdapter.h"

#include "TheoryHandle.h"
#include "klee/ADT/Ref.h"
#include "llvm/Support/Casting.h"

#include "klee/Support/ErrorHandling.h"

namespace klee {

struct Propositional : public SolverTheory<Propositional> {
public:
  Propositional(const ref<SolverAdapter> &adapter)
      : SolverTheory(SolverTheory::Sort::BOOL, adapter) {}

  template <typename... Args>
  ref<TheoryHandle<Propositional>> translate(const ref<Expr> &expr,
                                             Args &&...args) {
    switch (expr->getKind()) {
    case Expr::Kind::Constant: {
      return constant(expr);
    }
    case Expr::Kind::And: {
      return land(expr, args...);
    }
    case Expr::Kind::Or: {
      return lor(expr, args...);
    }
    case Expr::Kind::Xor: {
      return lxor(expr, args...);
    }
    case Expr::Kind::Not: {
      return lnot(expr, args...);
    }
    case Expr::Kind::Select: {
      return lite(expr, args...);
    }
    case Expr::Kind::Eq: {
      return eq(expr, args...);
    }
    default: {
      return nullptr;
    }
    }
  }

  std::string toString() const override { return "Boolean"; }

  ref<TheoryHandle<Propositional>> constant(const ref<ConstantExpr> &expr) {
    if (expr->getWidth() != 1) {
      return nullptr;
    }
    return new TheoryHandle<Propositional>(
        solverAdapter->propConst(expr->isTrue()), expr);
  }

  ref<TheoryHandle<Propositional>>
  land(ref<Expr> expr, const ref<TheoryHandle<Propositional>> &lhs,
       const ref<TheoryHandle<Propositional>> &rhs) {
    return new TheoryHandle<Propositional>(expr,
                                           solverAdapter->propAnd(lhs, rhs));
  }

  ref<TheoryHandle<Propositional>>
  lor(const ref<TheoryHandle<Propositional>> &lhs,
      const ref<TheoryHandle<Propositional>> &rhs) {
    return apply(std::bind(&SolverAdapter::propOr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<Propositional>>
  lxor(const ref<TheoryHandle<Propositional>> &lhs,
       const ref<TheoryHandle<Propositional>> &rhs) {
    return apply(std::bind(&SolverAdapter::propXor, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<Propositional>>
  lnot(const ref<TheoryHandle<Propositional>> &arg) {
    return apply(std::bind(&SolverAdapter::propNot, solverAdapter,
                           std::placeholders::_1),
                 arg);
  }

  template <typename RT>
  ref<TheoryHandle<RT>> lite(const ref<TheoryHandle<Propositional>> &cond,
                             const ref<TheoryHandle<RT>> &onTrue,
                             const ref<TheoryHandle<RT>> &onFalse) {
    ref<TheoryHandle> iteHandle = apply(
        std::bind(&SolverAdapter::propIte, solverAdapter, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        cond, onTrue, onFalse);
    iteHandle->parent = onTrue->parent;
    return iteHandle;
  }

  ref<TheoryHandle<Propositional>>
  eq(const ref<TheoryHandle<Propositional>> &lhs,
     const ref<TheoryHandle<Propositional>> &rhs) {
    return land(lhs, rhs);
  }
};

} // namespace klee
#endif
