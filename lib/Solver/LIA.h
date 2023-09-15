#ifndef LIA_H
#define LIA_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/Expr/Expr.h"

#include "klee/ADT/Ref.h"

namespace klee {

struct LIA : public SolverTheory {
public:
  LIA(ref<SolverAdapter> &adapter) : SolverTheory(adapter) {}

protected:
  virtual ref<TheoryResponse> translate(const ref<Expr> &expr,
                                        const ExprHandleList &args) {
    switch (expr->getKind()) {
    case Expr::Kind::Constant: {
      ref<ConstantExpr> ce = cast<ConstantExpr>(expr);
      if (ce->isFloat()) {
        return nullptr;
      }
      return constant(ce->getAPValue());
    }
    case Expr::Kind::Add: {
      return add(args[0], args[1]);
    }
    case Expr::Kind::Sub: {
      return sub(args[0], args[1]);
    }
    case Expr::Kind::Mul: {
      return mul(args[0], args[1]);
    }
    case Expr::Kind::SExt: {
      return sext(args[0], args[1]);
    }
    case Expr::Kind::ZExt: {
      return zext(args[0], args[1]);
    }
    case Expr::Kind::Ult: {
      return ult(args[0], args[1]);
    }
    case Expr::Kind::Ule: {
      return ule(args[0], args[1]);
    }
    case Expr::Kind::Slt: {
      return slt(args[0], args[1]);
    }
    case Expr::Kind::Sle: {
      return sle(args[0], args[1]);
    }
    default: {
      return nullptr;
    }
    }
  }

public:
  virtual ref<ExprHandle> constant(const llvm::APInt &value) {
    return solverAdapter->liaConst(value);
  }
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return solverAdapter->liaAdd(lhs, rhs);
  }
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return nullptr;
  }
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

} // namespace klee

#endif
