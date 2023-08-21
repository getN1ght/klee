#include "SolverTheory.h"

#include "SolverAdapter.h"

#include "klee/ADT/Ref.h"
#include "llvm/Support/Casting.h"

using namespace klee;

ref<ExprHandle> Propositional::translate(Expr::Kind kind,
                                            const ArgumentsList &args) {
  switch (kind) {
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
  default: {
    return nullptr;
  }
  }
}

ref<ExprHandle> Propositional::land(const ref<ExprHandle> &lhs,
                                    const ref<ExprHandle> &rhs) {
  return solverAdapter->propAnd(lhs, rhs);
}

ref<ExprHandle> Propositional::lor(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  return solverAdapter->propOr(lhs, rhs);
}

ref<ExprHandle> Propositional::lxor(const ref<ExprHandle> &lhs,
                                    const ref<ExprHandle> &rhs) {
  return solverAdapter->propXor(lhs, rhs);
}

ref<ExprHandle> Propositional::lnot(const ref<ExprHandle> &arg) {
  return solverAdapter->propNot(arg);
}

ref<ExprHandle> Propositional::lite(const ref<ExprHandle> &cond,
                                    const ref<ExprHandle> &onTrue,
                                    const ref<ExprHandle> &onFalse) {
  return solverAdapter->propIte(cond, onTrue, onFalse);
}
