#ifndef BV_H
#define BV_H

#include "SolverAdapter.h"
#include "SolverTheory.h"
#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "Propositional.h"

#include <iostream>

namespace klee {

/*
 * Theory of Bit Vectors. Supports all operations with
 * bit words available on modern machines.
 */
struct BV : public SolverTheory<BV> {
  friend class SolverTheory;

protected:

  template <typename T, typename... Args>
  ref<TheoryHandle<T>> translate(const ref<Expr> &expr,
                                 Args &&...args) {
    typedef Expr::Kind Kind;
    switch (expr->getKind()) {
    case Kind::Constant:
      return constant(expr);
    case Kind::Add:
      return add(expr, args...);
    case Kind::Sub:
      return sub(expr, args...);
    case Kind::Mul:
      return mul(expr, args...);
    case Kind::SDiv:
      return sdiv(expr, args...);
    case Kind::UDiv:
      return udiv(expr, args...);
    case Kind::Shl:
      return shl(expr, args...);
    case Kind::AShr:
      return ashr(expr, args...);
    case Kind::LShr:
      return lshr(expr, args...);
    case Kind::SExt:
      // FIXME: remove complex logic
      return sext(args[0],
                  constant(ConstantExpr::create(
                      expr->getWidth(), sizeof(expr->getWidth()) * CHAR_BIT)));
    case Kind::ZExt:
      // FIXME: remove complex logic
      return zext(args[0],
                  constant(ConstantExpr::create(
                      expr->getWidth(), sizeof(expr->getWidth()) * CHAR_BIT)));
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
      return extract(expr, args[0]);
    case Kind::Concat:
      return concat(args[0], args[1]);
    case Kind::Eq:
      return eq(args[0], args[1]);
    default:
      return new BrokenTheoryHandle(expr);
    }
  }

  // ref<TheoryHandle<Propositional>> castToBool(const ref<TheoryHandle<BV>> &handle) override {
  //   // SelectExpr::create()
  //   std::abort();
  //   return nullptr;
  // }

public:
  BV(const ref<SolverAdapter> &solverAdapter)
      : SolverTheory(SolverTheory::Sort::BV, solverAdapter) {}

  std::string toString() const override { return "BitVectors"; }

  ref<SortHandle> sort(unsigned width) { return solverAdapter->bvSort(width); }

  ref<TheoryHandle<BV>> constant(const ref<Expr> &val) {
    ref<ConstantExpr> cVal = dyn_cast<ConstantExpr>(val);
    if (!cVal) {
      // llvm::report_fatal_error()
      // TODO: REPORT FATAL ERROR
      std::abort();
    }
    return new CompleteTheoryHandle<BV>(
        solverAdapter->bvConst(cVal->getAPValue()), val);
  }

  template<typename ...Args>
  ref<TheoryHandle<BV>> add(const Args... &&args) {
    return new BrokenTheoryHandle<BV>(nullptr);
  }

  ref<TheoryHandle<BV>> add(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvAdd, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> sub(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSub, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> mul(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvMul, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> udiv(const ref<TheoryHandle<BV>> &lhs,
                             const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvUDiv, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> sdiv(const ref<TheoryHandle<BV>> &lhs,
                             const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSDiv, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> shl(const ref<TheoryHandle<BV>> &arg,
                        const ref<TheoryHandle<BV>> &wth) {
    return apply(std::bind(&SolverAdapter::bvShl, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle<BV>> ashr(const ref<TheoryHandle<BV>> &arg,
                             const ref<TheoryHandle<BV>> &wth) {
    return apply(std::bind(&SolverAdapter::bvAShr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle<BV>> lshr(const ref<TheoryHandle<BV>> &arg,
                             const ref<TheoryHandle<BV>> &wth) {
    return apply(std::bind(&SolverAdapter::bvLShr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle<BV>> sext(const ref<TheoryHandle<BV>> &arg,
                             const ref<TheoryHandle<BV>> &wth) {
    return apply(std::bind(&SolverAdapter::bvSExt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle<BV>> zext(const ref<TheoryHandle<BV>> &arg,
                             const ref<TheoryHandle<BV>> &wth) {
    return apply(std::bind(&SolverAdapter::bvZExt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 arg, wth);
  }

  ref<TheoryHandle<BV>> band(const ref<TheoryHandle<BV>> &lhs,
                             const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvAnd, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> bor(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvOr, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> bxor(const ref<TheoryHandle<BV>> &lhs,
                             const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvXor, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> bnot(const ref<TheoryHandle<BV>> &arg) {
    return apply(
        std::bind(&SolverAdapter::bvNot, solverAdapter, std::placeholders::_1),
        arg);
  }

  ref<TheoryHandle<BV>> ult(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvUlt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> ule(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvUle, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> slt(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSlt, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> sle(const ref<TheoryHandle<BV>> &lhs,
                            const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvSle, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<BV>> extract(const ref<Expr> &expr,
                                const ref<TheoryHandle<BV>> &arg) {
    ref<ExtractExpr> extractExpr = dyn_cast<ExtractExpr>(expr);
    return apply(
        std::bind(&SolverAdapter::bvExtract, solverAdapter,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        arg,
        constant(ConstantExpr::create(extractExpr->offset,
                                      sizeof(extractExpr->offset) * CHAR_BIT)),
        constant(
            ConstantExpr::create(extractExpr->getWidth(),
                                 sizeof(extractExpr->getWidth()) * CHAR_BIT)));
  }

  ref<TheoryHandle<BV>> concat(const ref<TheoryHandle<BV>> &lhs,
                               const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::bvConcat, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  ref<TheoryHandle<Propositional>> eq(const ref<TheoryHandle<BV>> &lhs,
                                      const ref<TheoryHandle<BV>> &rhs) {
    return apply(std::bind(&SolverAdapter::eq, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 lhs, rhs);
  }

  static bool classof(const SolverTheory *th) {
    return th->getSort() == Sort::BV;
  }
};

} // namespace klee

#endif
