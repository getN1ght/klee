#ifndef BV_H
#define BV_H

#include "SolverAdapter.h"
#include "SolverTheory.h"
#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "Propositional.h"

namespace klee {

/*
 * Theory of Bit Vectors. Supports all operations with
 * bit words available on modern machines.
 */
struct BV : public SolverTheory {
  friend class SolverTheory;

protected:
  ref<TheoryHandle> translate(const ref<Expr> &expr,
                              const TheoryHandleList &args) override {
    typedef Expr::Kind Kind;
    switch (expr->getKind()) {
    case Kind::Constant:
      return constant(expr);
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
    case Kind::Eq:
      return eq(args[0], args[1]);
    default:
      return nullptr;
    }
  }

  ref<TheoryHandle> castToBool(const ref<TheoryHandle> &handle) override {
    // SelectExpr::create()
    return nullptr;
  }

public:
  BV(const ref<SolverAdapter> &solverAdapter)
      : SolverTheory(SolverTheory::Sort::BV, solverAdapter) {}

  ref<SortHandle> sort(unsigned width) {
    return solverAdapter->bvSort(width);
  }

  ref<TheoryHandle> constant(const ref<Expr> &val) {
    ref<ConstantExpr> cVal = dyn_cast<ConstantExpr>(val);
    if (!cVal) {
      // llvm::report_fatal_error()
      // TODO: REPORT FATAL ERROR
      std::abort();
    }
    return new CompleteTheoryHandle(solverAdapter->bvConst(cVal->getAPValue()),
                                    this);
  }

  ref<TheoryHandle> add(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvAdd, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> sub(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSub, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> mul(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvMul, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> udiv(const ref<TheoryHandle> &lhs,
                         const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvUDiv, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> sdiv(const ref<TheoryHandle> &lhs,
                         const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSDiv, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> shl(const ref<TheoryHandle> &arg,
                        const ref<TheoryHandle> &wth) {
    return apply(std::bind(&SolverAdapter::bvShl, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle> ashr(const ref<TheoryHandle> &arg,
                         const ref<TheoryHandle> &wth) {
    return apply(std::bind(&SolverAdapter::bvAShr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle> lshr(const ref<TheoryHandle> &arg,
                         const ref<TheoryHandle> &wth) {
    return apply(std::bind(&SolverAdapter::bvLShr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle> sext(const ref<TheoryHandle> &arg,
                         const ref<TheoryHandle> &wth) {
    return apply(std::bind(&SolverAdapter::bvSExt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle> zext(const ref<TheoryHandle> &arg,
                         const ref<TheoryHandle> &wth) {
    return apply(std::bind(&SolverAdapter::bvZExt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle> band(const ref<TheoryHandle> &lhs,
                         const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvAnd, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> bor(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvOr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> bxor(const ref<TheoryHandle> &lhs,
                         const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvXor, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> bnot(const ref<TheoryHandle> &arg) {
    return apply(
        std::bind(&SolverAdapter::bvNot, solverAdapter, std::placeholders::_1),
        arg);
  }

  ref<TheoryHandle> ult(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvUlt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> ule(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvUle, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> slt(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSlt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> sle(const ref<TheoryHandle> &lhs,
                        const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSle, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> extract(const ref<TheoryHandle> &expr,
                            const ref<TheoryHandle> &off,
                            const ref<TheoryHandle> &len) {
    return apply(std::bind(&SolverAdapter::bvExtract, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3),
                 expr, off, len);
  }

  ref<TheoryHandle> concat(const ref<TheoryHandle> &lhs,
                           const ref<TheoryHandle> &rhs) {
    return apply(std::bind(&SolverAdapter::bvConcat, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle> eq(const ref<TheoryHandle> &lhs,
                       const ref<TheoryHandle> &rhs) {
    ref<TheoryHandle> eqHandle = apply(std::bind(&SolverAdapter::eq, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
    eqHandle->parent = new Propositional(solverAdapter);
    return eqHandle;
  }

  static bool classof(const SolverTheory *th) {
    return th->getSort() == Sort::BV;
  }
};

} // namespace klee

#endif
