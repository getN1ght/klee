#include "SolverTheory.h"

#include "SolverAdapter.h"

#include "klee/ADT/Ref.h"
#include "llvm/Support/Casting.h"

using namespace klee;

ref<ExprHandle> LIA::translate(Expr::Kind kind, const ArgumentsList &args) {
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

ref<ExprHandle> LIA::add(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->liaAdd(lhs, rhs);
}

ref<ExprHandle> LIA::sub(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->liaAdd(lhs, rhs);
}

ref<ExprHandle> LIA::mul(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  return solverAdapter->liaAdd(lhs, rhs);
}

ref<ExprHandle> LIA::sle(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  // return solverAdapter->liale(lhs, rhs);
  return nullptr;
}

ref<ExprHandle> LIA::slt(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  // return solverAdapter->liaLe(lhs, rhs);
  return nullptr;
}

ref<ExprHandle> LIA::ule(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  // return solverAdapter->liale(lhs, rhs);
  return nullptr;
}

ref<ExprHandle> LIA::ult(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) {
  // return solverAdapter->liaLe(lhs, rhs);
  return nullptr;
}

ref<ExprHandle> LIA::sext(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) {
  return nullptr;
}

ref<ExprHandle> LIA::zext(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) {
  return nullptr;
}
