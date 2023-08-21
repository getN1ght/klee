#include "SolverTheory.h"

#include "SolverAdapter.h"

#include <klee/ADT/Ref.h>

using namespace klee;

ref<ExprHandle> BV::translate(Expr::Kind kind, const ArgumentsList &args) {
  switch (kind) {
  case Expr::Kind::Add: {
    return add(args[0], args[1]);
  }
  case Expr::Kind::Sub: {
    return sub(args[0], args[1]);
  }
  case Expr::Kind::Mul: {
    return mul(args[0], args[1]);
  }
  case Expr::Kind::UDiv: {
    return udiv(args[0], args[1]);
  }
  case Expr::Kind::SDiv: {
    return sdiv(args[0], args[1]);
  }
  case Expr::Kind::Shl: {
    return shl(args[0], args[1]);
  }
  case Expr::Kind::AShr: {
    return ashr(args[0], args[1]);
  }
  case Expr::Kind::LShr: {
    return lshr(args[0], args[1]);
  }
  case Expr::Kind::SExt: {
    return sext(args[0], args[1]);
  }
  case Expr::Kind::ZExt: {
    return zext(args[0], args[1]);
  }
  case Expr::Kind::And: {
    return band(args[0], args[1]);
  }
  case Expr::Kind::Or: {
    return bor(args[0], args[1]);
  }
  case Expr::Kind::Xor: {
    return bxor(args[0], args[1]);
  }
  case Expr::Kind::Not: {
    return bnot(args[0]);
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
  case Expr::Kind::Extract: {
    return extract(args[0], args[1], args[2]);
  }
  default: {
    return nullptr;
  }
  }
}

ref<ExprHandle> BV::add(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvAdd(lhs, rhs);
}

ref<ExprHandle> BV::sub(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvSub(lhs, rhs);
}

ref<ExprHandle> BV::mul(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvMul(lhs, rhs);
}

ref<ExprHandle> BV::udiv(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->bvUDiv(lhs, rhs);
}

ref<ExprHandle> BV::sdiv(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->bvSDiv(lhs, rhs);
}

ref<ExprHandle> BV::band(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->bvAnd(lhs, rhs);
}

ref<ExprHandle> BV::bor(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvOr(lhs, rhs);
}

ref<ExprHandle> BV::bxor(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->bvXor(lhs, rhs);
}

ref<ExprHandle> BV::bnot(const ref<ExprHandle> &arg) {
  return solverAdapter->bvNot(arg);
}

ref<ExprHandle> BV::shl(const ref<ExprHandle> &arg,
                        const ref<ExprHandle> &wth) {
  return solverAdapter->bvShl(arg, wth);
}

ref<ExprHandle> BV::lshr(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &wth) {
  return solverAdapter->bvLShr(arg, wth);
}

ref<ExprHandle> BV::ashr(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &wth) {
  return solverAdapter->bvAShr(arg, wth);
}

ref<ExprHandle> BV::zext(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &wth) {
  return solverAdapter->bvZExt(arg, wth);
}

ref<ExprHandle> BV::sext(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &wth) {
  return solverAdapter->bvSExt(arg, wth);
}

ref<ExprHandle> BV::extract(const ref<ExprHandle> &arg,
                            const ref<ExprHandle> &off,
                            const ref<ExprHandle> &len) {
  return solverAdapter->bvExtract(arg, off, len);
}

ref<ExprHandle> BV::slt(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvSlt(lhs, rhs);
}

ref<ExprHandle> BV::sle(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvSle(lhs, rhs);
}

ref<ExprHandle> BV::ult(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvUlt(lhs, rhs);
}

ref<ExprHandle> BV::ule(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) {
  return solverAdapter->bvUle(lhs, rhs);
}
