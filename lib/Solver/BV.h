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
private:
  unsigned width;

protected:
  ref<ExprHandle> translate(const ref<Expr> &expr,
                            const ExprHandleList &args) override {
    // Strongly advise to check the number of arguments in arguments list.
    // TODO:
    // https://stackoverflow.com/questions/27024238/c-template-mechanism-to-get-the-number-of-function-arguments-which-would-work
    typedef Expr::Kind Kind;
    switch(expr->getKind()) {
    case Kind::Add:
      return add(args[0], args[1]);
    case Kind::Sub:
      return sub(args[0], args[1]);
    case Kind::Mul:
      return mul(args[0], args[1]);
    case Kind::SDiv:
      return sdiv(args[0], args[1]);
    case Kind::UDiv:
      return udiv(args[0], args[1]);
    case Kind::Shl:
      return shl(args[0], args[1]);
    case Kind::AShr:
      return ashr(args[0], args[1]);
    case Kind::LShr:
      return lshr(args[0], args[1]);
    case Kind::SExt:
      return sext(args[0], args[1]);
    case Kind::ZExt:
      return zext(args[0], args[1]);
    case Kind::And:
      return band(args[0], args[1]);
    case Kind::Or:
      return bor(args[0], args[1]);
    case Kind::Xor:
      return bxor(args[0], args[1]);
    case Kind::Not:
      return bnot(args[0]);
    case Kind::Ule:
      return ule(args[0], args[1]);
    case Kind::Ult:
      return ult(args[0], args[1]);
    case Kind::Sle:
      return sle(args[0], args[1]);
    case Kind::Slt:
      return slt(args[0], args[1]);
    case Kind::Extract:
      return extract(args[0], args[1], args[2]);
    case Kind::Concat:
      return concat(args[0], args[1]);
    default:
      return nullptr;
    }
  }

public:
  BV(uint32_t width) : width(width) {}

  ref<ExprHandle> sort() override {
    // TODO: how to choose width of bitvector?
    return solverAdapter->bv(width);
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