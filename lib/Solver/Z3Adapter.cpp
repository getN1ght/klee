#include "Z3Adapter.h"

#include "SolverBuilder.h"
#include "z3++.h"

#include "llvm/Support/Casting.h"

using namespace klee;

Z3ExprHandle::Z3ExprHandle(const z3::expr &node) : expr(node) {}

Z3Adapter::Z3Adapter() : ctx(z3::context()) {}

ref<ExprHandle> Z3Adapter::bvAdd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle((z3::expr)*lhsZ3.get() + (z3::expr)*rhsZ3.get());
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSub(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle((z3::expr)*lhsZ3.get() - (z3::expr)*rhsZ3.get());
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvMul(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 * (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvUDiv(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle(z3::udiv((z3::expr)*lhsZ3, (z3::expr)*rhsZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSDiv(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 / (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvAnd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 & (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvOr(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 & (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvXor(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 ^ (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvNot(const ref<ExprHandle> &arg) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  ExprHandle *handle = new Z3ExprHandle(~(z3::expr)*argZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSle(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle(z3::sle((z3::expr)*lhsZ3, (z3::expr)*rhsZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSlt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle(z3::slt((z3::expr)*lhsZ3, (z3::expr)*rhsZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvUle(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle(z3::ule((z3::expr)*lhsZ3, (z3::expr)*rhsZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvUlt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle =
      new Z3ExprHandle(z3::ult((z3::expr)*lhsZ3, (z3::expr)*rhsZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvZExt(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle =
      new Z3ExprHandle(z3::zext((z3::expr)*argZ3, (z3::expr)*widthZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSExt(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle =
      new Z3ExprHandle(z3::sext((z3::expr)*argZ3, (z3::expr)*widthZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvShl(const ref<ExprHandle> &arg,
                                 const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle =
      new Z3ExprHandle(z3::shl((z3::expr)*argZ3, (z3::expr)*widthZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvLShr(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle =
      new Z3ExprHandle(z3::lshr((z3::expr)*argZ3, (z3::expr)*widthZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvAShr(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle =
      new Z3ExprHandle(z3::ashr((z3::expr)*argZ3, (z3::expr)*widthZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvExtract(const ref<ExprHandle> &expr,
                                     const ref<ExprHandle> &off,
                                     const ref<ExprHandle> &len) {
  const ref<Z3ExprHandle> exprZ3 = cast<Z3ExprHandle>(expr);
  const ref<Z3ExprHandle> offZ3 = cast<Z3ExprHandle>(off);
  const ref<Z3ExprHandle> lenZ3 = cast<Z3ExprHandle>(len);
  ExprHandle *handle = new Z3ExprHandle(static_cast<z3::expr>(*exprZ3).extract(
      (z3::expr)*offZ3, (z3::expr)*lenZ3));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propAnd(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 && (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propOr(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 || (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propXor(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle((z3::expr)*lhsZ3 ^ (z3::expr)*rhsZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propNot(const ref<ExprHandle> &arg) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  ExprHandle *handle = new Z3ExprHandle(!(z3::expr)*argZ3);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propIte(const ref<ExprHandle> &cond,
                                   const ref<ExprHandle> &onTrue,
                                   const ref<ExprHandle> &onFalse) {
  const ref<Z3ExprHandle> condZ3 = cast<Z3ExprHandle>(cond);
  const ref<Z3ExprHandle> onTrueZ3 = cast<Z3ExprHandle>(onTrue);
  const ref<Z3ExprHandle> onFalseZ3 = cast<Z3ExprHandle>(onFalse);
  ExprHandle *handle = new Z3ExprHandle(
      z3::ite((z3::expr)*condZ3, (z3::expr)*onTrueZ3, (z3::expr)*onFalseZ3));
  return ref<ExprHandle>(handle);
}
