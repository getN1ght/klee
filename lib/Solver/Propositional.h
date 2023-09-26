#ifndef PROPOSITIONAL_H
#define PROPOSITIONAL_H

#include "SolverTheory.h"

#include "SolverAdapter.h"

#include "klee/ADT/Ref.h"
#include "llvm/Support/Casting.h"

#include "klee/Support/ErrorHandling.h"

namespace klee {

struct Propositional : public SolverTheory<Propositional> {

protected:
  ref<TheoryHandle> castToBV(const ref<TheoryHandle> &arg) override {
    static const llvm::APInt zero(1, 0);
    static const llvm::APInt one(1, 1);
    return lite(arg, solverAdapter->bvConst(one), solverAdapter->bvConst(zero));
  }

  ref<TheoryHandle> castToLIA(const ref<TheoryHandle> &arg) override {
    static const llvm::APInt zero(1, 0);
    static const llvm::APInt one(1, 1);
    return lite(arg, solverAdapter->liaConst(one),
                solverAdapter->liaConst(zero));
  }

public:
  Propositional(const ref<SolverAdapter> &adapter)
      : SolverTheory(SolverTheory::Sort::BOOL, adapter) {}

  ref<TheoryHandle> translate(const ref<Expr> &expr,
                              const TheoryHandleList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Constant: {
      return constant(expr);
    }
    case Expr::Kind::And: {
      return land(args[0], args[1]);
    }
    case Expr::Kind::Or: {
      return lor(args[0], args[1]);
    }
    case Expr::Kind::Xor: {
      return lxor(args[0], args[1]);
    }
    case Expr::Kind::Not: {
      return lnot(args[0]);
    }
    case Expr::Kind::Select: {
      return lite(args[0], args[1], args[2]);
    }
    case Expr::Kind::Eq: {
      return eq(args[0], args[1]);
    }
    default: {
      return new BrokenTheoryHandle(expr);
    }
    }
  }

  std::string toString() const override { return "Boolean"; }

  ref<TheoryHandle<Propositional>> constant(const ref<Expr> &expr) {
    ref<ConstantExpr> ce = cast<ConstantExpr>(expr);
    if (ce->getWidth() != 1) {
      return new BrokenTheoryHandle<Propositional>(expr);
    }
    return new CompleteTheoryHandle<Propositional>(
        solverAdapter->propConst(ce->isTrue()), this);
  }

  ref<TheoryHandle<Propositional>> land(const ref<TheoryHandle<Propositional>> &lhs,
                         const ref<TheoryHandle<Propositional>> &rhs) {
    return apply(std::bind(std::mem_fn(&SolverAdapter::propAnd), solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<Propositional>> lor(const ref<TheoryHandle<Propositional>> &lhs,
                        const ref<TheoryHandle<Propositional>> &rhs) {
    return apply(std::bind(&SolverAdapter::propOr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<Propositional>> lxor(const ref<TheoryHandle<Propositional>> &lhs,
                         const ref<TheoryHandle<Propositional>> &rhs) {
    return apply(std::bind(&SolverAdapter::propXor, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<Propositional>> lnot(const ref<TheoryHandle<Propositional>> &arg) {
    return apply(std::bind(&SolverAdapter::propNot, solverAdapter,
                           std::placeholders::_1),
                 arg);
  }

  template<typename RT>
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
