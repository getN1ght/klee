#include "Z3Adapter.h"

#include "SolverBuilder.h"
#include "z3++.h"

#include "llvm/ADT/APInt.h"
#include "llvm/Support/Casting.h"

using namespace klee;

Z3ExprHandle::Z3ExprHandle(const z3::expr &node) : expr(node) {}

Z3Adapter::Z3Adapter() : ctx(z3::context()) {}

ref<ExprHandle> Z3Adapter::array(const ref<ExprHandle> &domainSort,
                                 const ref<ExprHandle> &rangeSort) {
  const ref<Z3ExprHandle> &domainSortZ3 = cast<Z3ExprHandle>(domainSort);
  const ref<Z3ExprHandle> &rangeSortZ3 = cast<Z3ExprHandle>(rangeSort);
  std::abort();
  // return ctx.array_sort();
  return nullptr;
}

ref<ExprHandle> Z3Adapter::bv(uint64_t width) {
  unsigned int bvWidth = static_cast<unsigned int>(width);
  z3::sort sort = ctx.bv_sort(bvWidth);
  std::abort();
  // TODO:
  return nullptr;
}

ref<ExprHandle> Z3Adapter::bvConst(const llvm::APInt &val) {
  unsigned int valBitWidth = val.getBitWidth(); 
  // FIXME: Which length?
  llvm::SmallString<128> valStringRepresentation;
  val.toStringUnsigned(valStringRepresentation);
  z3::expr valExpr = ctx.bv_val(valStringRepresentation.c_str(), valBitWidth);
  ExprHandle *handle = new Z3ExprHandle(valExpr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvAdd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr + rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSub(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr - rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvMul(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr * rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvUDiv(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(z3::udiv(lhsZ3->expr, rhsZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSDiv(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr / rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvAnd(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr & rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvOr(const ref<ExprHandle> &lhs,
                                const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr & rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvXor(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr ^ rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvNot(const ref<ExprHandle> &arg) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  ExprHandle *handle = new Z3ExprHandle(~argZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSle(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(z3::sle(lhsZ3->expr, rhsZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSlt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(z3::slt(lhsZ3->expr, rhsZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvUle(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(z3::ule(lhsZ3->expr, rhsZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvUlt(const ref<ExprHandle> &lhs,
                                 const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(z3::ult(lhsZ3->expr, rhsZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvZExt(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle = new Z3ExprHandle(z3::zext(argZ3->expr, widthZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvSExt(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle = new Z3ExprHandle(z3::sext(argZ3->expr, widthZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvShl(const ref<ExprHandle> &arg,
                                 const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle = new Z3ExprHandle(z3::shl(argZ3->expr, widthZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvLShr(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle = new Z3ExprHandle(z3::lshr(argZ3->expr, widthZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvAShr(const ref<ExprHandle> &arg,
                                  const ref<ExprHandle> &width) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  const ref<Z3ExprHandle> widthZ3 = cast<Z3ExprHandle>(width);
  ExprHandle *handle = new Z3ExprHandle(z3::ashr(argZ3->expr, widthZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvExtract(const ref<ExprHandle> &expr,
                                     const ref<ExprHandle> &off,
                                     const ref<ExprHandle> &len) {
  const ref<Z3ExprHandle> exprZ3 = cast<Z3ExprHandle>(expr);
  const ref<Z3ExprHandle> offZ3 = cast<Z3ExprHandle>(off);
  const ref<Z3ExprHandle> lenZ3 = cast<Z3ExprHandle>(len);
  ExprHandle *handle =
      new Z3ExprHandle(exprZ3->expr.extract(offZ3->expr, lenZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::bvConcat(const ref<ExprHandle> &lhs,
                                    const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(z3::concat(lhsZ3->expr, rhsZ3->expr));
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propAnd(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr && rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propOr(const ref<ExprHandle> &lhs,
                                  const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr || rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propXor(const ref<ExprHandle> &lhs,
                                   const ref<ExprHandle> &rhs) {
  const ref<Z3ExprHandle> lhsZ3 = cast<Z3ExprHandle>(lhs);
  const ref<Z3ExprHandle> rhsZ3 = cast<Z3ExprHandle>(rhs);
  ExprHandle *handle = new Z3ExprHandle(lhsZ3->expr ^ rhsZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propNot(const ref<ExprHandle> &arg) {
  const ref<Z3ExprHandle> argZ3 = cast<Z3ExprHandle>(arg);
  ExprHandle *handle = new Z3ExprHandle(!argZ3->expr);
  return ref<ExprHandle>(handle);
}

ref<ExprHandle> Z3Adapter::propIte(const ref<ExprHandle> &cond,
                                   const ref<ExprHandle> &onTrue,
                                   const ref<ExprHandle> &onFalse) {
  const ref<Z3ExprHandle> condZ3 = cast<Z3ExprHandle>(cond);
  const ref<Z3ExprHandle> onTrueZ3 = cast<Z3ExprHandle>(onTrue);
  const ref<Z3ExprHandle> onFalseZ3 = cast<Z3ExprHandle>(onFalse);
  ExprHandle *handle =
      new Z3ExprHandle(z3::ite(condZ3->expr, onTrueZ3->expr, onFalseZ3->expr));
  return ref<ExprHandle>(handle);
}
