#include "SolverAdapter.h"

#include "klee/ADT/Ref.h"

#include <optional>

using namespace klee;

ref<ExprHandle> SolverAdapter::Arrays::applyHandler(Expr::Kind kind,
                                                    const ArgumentsList &args) {
  switch (kind) {
  case Expr::Kind::Read: {
    return read(args[0], args[1]);
  }
  default: {
    return nullptr;
  }
  }
}

ref<ExprHandle> SolverAdapter::BV::applyHandler(Expr::Kind kind,
                                                const ArgumentsList &args) {
  switch (kind) {
  case Expr::Kind::Add: {
    return (add(args[0], args[1]));
  }
  case Expr::Kind::Sub: {
    return (sub(args[0], args[1]));
  }
  case Expr::Kind::Mul: {
    return (mul(args[0], args[1]));
  }
  case Expr::Kind::UDiv: {
    return (udiv(args[0], args[1]));
  }
  case Expr::Kind::SDiv: {
    return (sdiv(args[0], args[1]));
  }
  case Expr::Kind::Shl: {
    return (shl(args[0], args[1]));
  }
  case Expr::Kind::AShr: {
    return (ashr(args[0], args[1]));
  }
  case Expr::Kind::LShr: {
    return (lshr(args[0], args[1]));
  }
  case Expr::Kind::SExt: {
    return (sext(args[0], args[1]));
  }
  case Expr::Kind::ZExt: {
    return (zext(args[0], args[1]));
  }
  case Expr::Kind::And: {
    return (band(args[0], args[1]));
  }
  case Expr::Kind::Or: {
    return (bor(args[0], args[1]));
  }
  case Expr::Kind::Xor: {
    return (bxor(args[0], args[1]));
  }
  case Expr::Kind::Not: {
    return (bnot(args[0]));
  }
  case Expr::Kind::Ult: {
    return (ult(args[0], args[1]));
  }
  case Expr::Kind::Ule: {
    return (ule(args[0], args[1]));
  }
  case Expr::Kind::Slt: {
    return (slt(args[0], args[1]));
  }
  case Expr::Kind::Sle: {
    return (sle(args[0], args[1]));
  }
  case Expr::Kind::Extract: {
    return (extract());
  }
  default: {
    return nullptr;
  }
  }
}

ref<ExprHandle> SolverAdapter::LIA::applyHandler(Expr::Kind kind,
                                                 const ArgumentsList &args) {
  switch (kind) {
  case Expr::Kind::Add: {
    return (add(args[0], args[1]));
  }
  case Expr::Kind::Sub: {
    return (sub(args[0], args[1]));
  }
  case Expr::Kind::Mul: {
    return (mul(args[0], args[1]));
  }
  case Expr::Kind::SExt: {
    return (sext(args[0], args[1]));
  }
  case Expr::Kind::ZExt: {
    return (zext(args[0], args[1]));
  }
  case Expr::Kind::Ult: {
    return (ult(args[0], args[1]));
  }
  case Expr::Kind::Ule: {
    return (ule(args[0], args[1]));
  }
  case Expr::Kind::Slt: {
    return (slt(args[0], args[1]));
  }
  case Expr::Kind::Sle: {
    return (sle(args[0], args[1]));
  }
  default: {
    return nullptr;
  }
  }
}

ref<ExprHandle>
SolverAdapter::Propositional::applyHandler(Expr::Kind kind,
                                           const ArgumentsList &args) {
  switch (kind) {
  case Expr::Kind::And: {
    return (land(args[0], args[1]));
  }
  case Expr::Kind::Or: {
    return (lor(args[0], args[1]));
  }
  case Expr::Kind::Xor: {
    return (lxor(args[0], args[1]));
  }
  case Expr::Kind::Not: {
    return (lnot(args[0]));
  }
  case Expr::Kind::Select: {
    return (lite(args[0], args[1], args[2]));
  }
  default: {
    return nullptr;
  }
  }
}
