#ifndef LIA_H
#define LIA_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/Expr/Expr.h"

#include "klee/ADT/Ref.h"

namespace klee {

struct LIA : public SolverTheory {
public:
  LIA(ref<SolverAdapter> &adapter)
      : SolverTheory(SolverTheory::Sort::LIA, adapter) {}

protected:
  virtual ref<TheoryHandle> translate(const ref<Expr> &expr,
                                      const TheoryHandleList &args) {
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
  std::string toString() const override {
    return "Linear Integer Arithmetic"; 
  }

  virtual ref<TheoryHandle> constant(const llvm::APInt &value) {
    return solverAdapter->liaConst(value);
  }
  virtual ref<TheoryHandle> add(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    return solverAdapter->liaAdd(lhs, rhs);
  }
  virtual ref<TheoryHandle> sub(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    // return solverAdapter->lia;
    return nullptr;
  }
  virtual ref<TheoryHandle> mul(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    return solverAdapter->liaMul(lhs, rhs);
  }

  virtual ref<TheoryHandle> sext(const ref<TheoryHandle> &lhs,
                                 const ref<TheoryHandle> &rhs) {
    return nullptr;
  }
  virtual ref<TheoryHandle> zext(const ref<TheoryHandle> &lhs,
                                 const ref<TheoryHandle> &rhs) {
    return nullptr;
  }

  virtual ref<TheoryHandle> ult(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    return nullptr;
  }

  virtual ref<TheoryHandle> ule(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    return nullptr;
  }

  virtual ref<TheoryHandle> slt(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    return nullptr;
  }
  virtual ref<TheoryHandle> sle(const ref<TheoryHandle> &lhs,
                                const ref<TheoryHandle> &rhs) {
    return nullptr;
  }
};

} // namespace klee

#endif
