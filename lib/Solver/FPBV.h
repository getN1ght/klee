#ifndef FPBV_H
#define FPBV_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "llvm/ADT/APFloat.h"

namespace klee {
struct FPBV : public SolverTheory {
public:
  FPBV(const ref<SolverAdapter> &adapter) : SolverTheory(adapter) {}

protected:
  ref<TheoryHandle> translate(const ref<Expr> &expr,
                                const ExprHandleList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Constant: {
      return apply(&FPBV::constant, expr);
    }
    case Expr::FAdd: {
      return apply(&FPBV::add, args[0], args[1]);
    }
    case Expr::FSub: {
      return apply(&FPBV::sub, args[0], args[1]);
    }
    case Expr::FMul: {
      return apply(&FPBV::mul, args[0], args[1]);
    }
    case Expr::FDiv: {
      return apply(&FPBV::div, args[0], args[1]);
    }
    case Expr::FRem: {
      return apply(&FPBV::rem, args[0], args[1]);
    }
    case Expr::FMin: {
      return apply(&FPBV::fmin, args[0], args[1]);
    }
    case Expr::FMax: {
      return apply(&FPBV::fmax, args[0], args[1]);
    }
    case Expr::FOLe: {
      return apply(&FPBV::le, args[0], args[1]);
    }
    case Expr::FOLt: {
      return apply(&FPBV::lt, args[0], args[1]);
    }
    case Expr::FOEq: {
      return apply(&FPBV::eq, args[0], args[1]);
    }
    case Expr::FNeg: {
      return apply(&FPBV::neg, args[0]);
    }
    case Expr::IsInfinite: {
      return apply(&FPBV::isInf, args[0]);
    }
    case Expr::IsNormal: {
      return apply(&FPBV::isNormal, args[0]);
    }
    case Expr::IsSubnormal: {
      return apply(&FPBV::isSubNormal, args[0]);
    }
    case Expr::IsNaN: {
      return apply(&FPBV::isNaN, args[0]);
    }
    case Expr::FSqrt: {
      return apply(&FPBV::sqrt, args[0]);
    }
    case Expr::FRint: {
      return apply(&FPBV::rint, args[0]);
    }
    case Expr::FAbs: {
      return apply(&FPBV::abs, args[0]);
    }
    default: {
      return nullptr;
    }
    }
  }

public:
  virtual ref<ExprHandle> constant(const ref<Expr> &expr) {
    ref<ConstantExpr> ce = cast<ConstantExpr>(expr);
    if (!ce->isFloat()) {
      return nullptr;
    }
    return solverAdapter->bvFConst(ce->getAPFloatValue());
  }
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFAdd(lhs, rhs);
  }
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFSub(lhs, rhs);
  }
  virtual ref<ExprHandle> mul(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFMul(lhs, rhs);
  }
  virtual ref<ExprHandle> div(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFDiv(lhs, rhs);
  }
  virtual ref<ExprHandle> rem(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFRem(lhs, rhs);
  }
  virtual ref<ExprHandle> neg(const ref<ExprHandle> &expr) {
    return solverAdapter->bvFNeg(expr);
  }
  virtual ref<ExprHandle> isInf(const ref<ExprHandle> &expr) {
    return solverAdapter->bvFIsInfinite(expr);
  }
  virtual ref<ExprHandle> isNaN(const ref<ExprHandle> &expr) {
    return solverAdapter->bvFIsNaN(expr);
  }
  virtual ref<ExprHandle> isNormal(const ref<ExprHandle> &expr) {
    return solverAdapter->bvFIsNormal(expr);
  }
  virtual ref<ExprHandle> isSubNormal(const ref<ExprHandle> &expr) {
    return solverAdapter->bvFIsSubnormal(expr);
  }
  virtual ref<ExprHandle> fmax(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFMax(lhs, rhs);
  }
  virtual ref<ExprHandle> fmin(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFMin(lhs, rhs);
  }
  virtual ref<ExprHandle> lt(const ref<ExprHandle> &lhs,
                             const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFOLt(lhs, rhs);
  }
  virtual ref<ExprHandle> le(const ref<ExprHandle> &lhs,
                             const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFOLe(lhs, rhs);
  }
  virtual ref<ExprHandle> eq(const ref<ExprHandle> &lhs,
                             const ref<ExprHandle> &rhs) {
    return solverAdapter->bvFOEq(lhs, rhs);
  }
  virtual ref<ExprHandle> sqrt(const ref<ExprHandle> &arg) {
    return solverAdapter->bvFSqrt(arg);
  }
  virtual ref<ExprHandle> rint(const ref<ExprHandle> &arg) {
    return solverAdapter->bvFRint(arg);
  }
  virtual ref<ExprHandle> abs(const ref<ExprHandle> &arg) {
    return solverAdapter->bvFAbs(arg);
  }
};
} // namespace klee

#endif