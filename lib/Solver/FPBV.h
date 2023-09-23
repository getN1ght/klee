#ifndef FPBV_H
#define FPBV_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "llvm/ADT/APFloat.h"

namespace klee {
struct FPBV : public SolverTheory {
public:
  FPBV(const ref<SolverAdapter> &adapter)
      : SolverTheory(SolverTheory::Sort::FPBV, adapter) {}

protected:
  ref<TheoryHandle> translate(const ref<Expr> &expr,
                                const TheoryHandleList &args) override {
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
  std::string toString() const override {
    return "Floating Point BitVectors"; 
  }

  virtual ref<SolverHandle> constant(const ref<Expr> &expr) {
    ref<ConstantExpr> ce = cast<ConstantExpr>(expr);
    if (!ce->isFloat()) {
      return nullptr;
    }
    return solverAdapter->bvFConst(ce->getAPFloatValue());
  }
  virtual ref<SolverHandle> add(const ref<SolverHandle> &lhs,
                              const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFAdd(lhs, rhs);
  }
  virtual ref<SolverHandle> sub(const ref<SolverHandle> &lhs,
                              const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFSub(lhs, rhs);
  }
  virtual ref<SolverHandle> mul(const ref<SolverHandle> &lhs,
                              const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFMul(lhs, rhs);
  }
  virtual ref<SolverHandle> div(const ref<SolverHandle> &lhs,
                              const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFDiv(lhs, rhs);
  }
  virtual ref<SolverHandle> rem(const ref<SolverHandle> &lhs,
                              const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFRem(lhs, rhs);
  }
  virtual ref<SolverHandle> neg(const ref<SolverHandle> &expr) {
    return solverAdapter->bvFNeg(expr);
  }
  virtual ref<SolverHandle> isInf(const ref<SolverHandle> &expr) {
    return solverAdapter->bvFIsInfinite(expr);
  }
  virtual ref<SolverHandle> isNaN(const ref<SolverHandle> &expr) {
    return solverAdapter->bvFIsNaN(expr);
  }
  virtual ref<SolverHandle> isNormal(const ref<SolverHandle> &expr) {
    return solverAdapter->bvFIsNormal(expr);
  }
  virtual ref<SolverHandle> isSubNormal(const ref<SolverHandle> &expr) {
    return solverAdapter->bvFIsSubnormal(expr);
  }
  virtual ref<SolverHandle> fmax(const ref<SolverHandle> &lhs,
                               const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFMax(lhs, rhs);
  }
  virtual ref<SolverHandle> fmin(const ref<SolverHandle> &lhs,
                               const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFMin(lhs, rhs);
  }
  virtual ref<SolverHandle> lt(const ref<SolverHandle> &lhs,
                             const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFOLt(lhs, rhs);
  }
  virtual ref<SolverHandle> le(const ref<SolverHandle> &lhs,
                             const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFOLe(lhs, rhs);
  }
  virtual ref<SolverHandle> eq(const ref<SolverHandle> &lhs,
                             const ref<SolverHandle> &rhs) {
    return solverAdapter->bvFOEq(lhs, rhs);
  }
  virtual ref<SolverHandle> sqrt(const ref<SolverHandle> &arg) {
    return solverAdapter->bvFSqrt(arg);
  }
  virtual ref<SolverHandle> rint(const ref<SolverHandle> &arg) {
    return solverAdapter->bvFRint(arg);
  }
  virtual ref<SolverHandle> abs(const ref<SolverHandle> &arg) {
    return solverAdapter->bvFAbs(arg);
  }
};
} // namespace klee

#endif