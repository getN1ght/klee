#ifndef BV_H
#define BV_H

#include "SolverAdapter.h"
#include "SolverTheory.h"
#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

namespace klee {

/*
 * Theory of Bit Vectors. Supports all operations with
 * bit words available on modern machines.
 */
struct BV : public SolverTheory {
  friend class SolverTheory;

protected:
  ref<TheoryResponse> translate(const ref<Expr> &expr,
                                const ExprHandleList &args) override {
    typedef Expr::Kind Kind;
    switch (expr->getKind()) {
    case Kind::Constant:
      return constant(expr);
    case Kind::Add:
      return apply(&BV::add, args[0], args[1]);
    case Kind::Sub:
      return apply(&BV::sub, args[0], args[1]);
    case Kind::Mul:
      return apply(&BV::mul, args[0], args[1]);
    case Kind::SDiv:
      return apply(&BV::sdiv, args[0], args[1]);
    case Kind::UDiv:
      return apply(&BV::udiv, args[0], args[1]);
    case Kind::Shl:
      return apply(&BV::shl, args[0], args[1]);
    case Kind::AShr:
      return apply(&BV::ashr, args[0], args[1]);
    case Kind::LShr:
      return apply(&BV::lshr, args[0], args[1]);
    case Kind::SExt:
      return apply(&BV::sext, args[0], args[1]);
    case Kind::ZExt:
      return apply(&BV::zext, args[0], args[1]);
    case Kind::And:
      return apply(&BV::band, args[0], args[1]);
    case Kind::Or:
      return apply(&BV::bor, args[0], args[1]);
    case Kind::Xor:
      return apply(&BV::bxor, args[0], args[1]);
    case Kind::Not:
      return apply(&BV::bnot, args[0]);
    case Kind::Ule:
      return apply(&BV::ule, args[0], args[1]);
    case Kind::Ult:
      return apply(&BV::ult, args[0], args[1]);
    case Kind::Sle:
      return apply(&BV::sle, args[0], args[1]);
    case Kind::Slt:
      return apply(&BV::slt, args[0], args[1]);
    case Kind::Extract:
      return apply(&BV::extract, args[0], args[1], args[2]);
    case Kind::Concat:
      return apply(&BV::concat, args[0], args[1]);
    default:
      return nullptr;
    }
  }

  ref<ExprHandle> castToBool(const ref<ExprHandle> &handle) override {
    return nullptr;
  }

public:
  BV(const ref<SolverAdapter> &solverAdapter) : SolverTheory(solverAdapter) {}

  ref<ExprHandle> sort(unsigned width) {
    return solverAdapter->bvSort(width);
  }

  ref<ExprHandle> constant(const ref<Expr> &val) {
    ref<ConstantExpr> cVal = dyn_cast<ConstantExpr>(val);
    if (!cVal) {
      // llvm::report_fatal_error()
      // TODO: REPORT FATAL ERROR
      std::abort();
    }
    return solverAdapter->bvConst(cVal->getAPValue());
  }

  ref<ExprHandle> add(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvAdd(lhs, rhs);
  }

  ref<ExprHandle> sub(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvSub(lhs, rhs);
  }
  ref<ExprHandle> mul(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvMul(lhs, rhs);
  }
  ref<ExprHandle> udiv(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvUDiv(lhs, rhs);
  }
  ref<ExprHandle> sdiv(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvSDiv(lhs, rhs);
  }

  ref<ExprHandle> shl(const ref<ExprHandle> &arg, const ref<ExprHandle> &wth) {
    return solverAdapter->bvShl(arg, wth);
  }
  ref<ExprHandle> ashr(const ref<ExprHandle> &arg, const ref<ExprHandle> &wth) {
    return solverAdapter->bvAShr(arg, wth);
  }
  ref<ExprHandle> lshr(const ref<ExprHandle> &arg, const ref<ExprHandle> &wth) {
    return solverAdapter->bvLShr(arg, wth);
  }

  ref<ExprHandle> sext(const ref<ExprHandle> &arg, const ref<ExprHandle> &wth) {
    return solverAdapter->bvSExt(arg, wth);
  }

  ref<ExprHandle> zext(const ref<ExprHandle> &arg, const ref<ExprHandle> &wth) {
    return solverAdapter->bvZExt(arg, wth);
  }

  ref<ExprHandle> band(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvAnd(lhs, rhs);
  }
  ref<ExprHandle> bor(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvOr(lhs, rhs);
  }
  ref<ExprHandle> bxor(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvXor(lhs, rhs);
  }

  ref<ExprHandle> bnot(const ref<ExprHandle> &arg) {
    return solverAdapter->bvNot(arg);
  }

  ref<ExprHandle> ult(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvUlt(lhs, rhs);
  }

  ref<ExprHandle> ule(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvUle(lhs, rhs);
  }

  ref<ExprHandle> slt(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvSlt(lhs, rhs);
  }
  ref<ExprHandle> sle(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs) {
    return solverAdapter->bvSle(lhs, rhs);
  }

  ref<ExprHandle> extract(const ref<ExprHandle> &expr,
                          const ref<ExprHandle> &off,
                          const ref<ExprHandle> &len) {
    return solverAdapter->bvExtract(expr, off, len);
  }

  ref<ExprHandle> concat(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
    return solverAdapter->bvConcat(lhs, rhs);
  }
};

} // namespace klee

#endif