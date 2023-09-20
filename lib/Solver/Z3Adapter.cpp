#include "Z3Adapter.h"

#include "SolverBuilder.h"
#include "z3++.h"

#include "llvm/ADT/APInt.h"
#include "llvm/Support/Casting.h"

using namespace klee;

Z3SolverHandle::Z3SolverHandle(const z3::expr &node) : expr(node) {}
Z3SortSolverHandle::Z3SortSolverHandle(const z3::sort &sort) : sort(sort) {}

ref<SortHandle> Z3Adapter::arraySort(const ref<SortHandle> &domainSort,
                                     const ref<SortHandle> &rangeSort) {
  const ref<Z3SortSolverHandle> domainSortZ3 =
      cast<Z3SortSolverHandle>(domainSort);
  const ref<Z3SortSolverHandle> rangeSortZ3 =
      cast<Z3SortSolverHandle>(rangeSort);
  return new Z3SortSolverHandle(
      ctx.array_sort(domainSortZ3->sort, rangeSortZ3->sort));
}

ref<SortHandle> Z3Adapter::bvSort(uint64_t width) {
  unsigned int bvWidth = static_cast<unsigned int>(width);
  return new Z3SortSolverHandle(ctx.bv_sort(bvWidth));
}

ref<SortHandle> Z3Adapter::liaSort() {
  return new Z3SortSolverHandle(ctx.int_sort());
}

ref<SortHandle> Z3Adapter::boolSort() {
  return new Z3SortSolverHandle(ctx.bool_sort());
}

ref<SolverHandle> Z3Adapter::eq(const ref<SolverHandle> &lhs,
                                const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr == rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

// ref<SolverHandle> Z3Adapter::bvConst(uint64_t width) {
//   static uint64_t id = 0;

//   unsigned int bvWidth = static_cast<unsigned int>(width);
//   SolverHandle *handle = new Z3SolverHandle(ctx.bv_const(
//       (std::string("bv#") + std::to_string(id++)).c_str(), bvWidth));
//   return ref<SolverHandle>(handle);
// }

ref<SolverHandle> Z3Adapter::bvConst(const llvm::APInt &val) {
  unsigned int valBitWidth = val.getBitWidth();
  // FIXME: Which length?
  llvm::SmallString<128> valStringRepresentation;
  val.toStringUnsigned(valStringRepresentation);
  z3::expr valExpr = ctx.bv_val(valStringRepresentation.c_str(), valBitWidth);
  SolverHandle *handle = new Z3SolverHandle(valExpr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvAdd(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr + rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvSub(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr - rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvMul(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr * rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvUDiv(const ref<SolverHandle> &lhs,
                                    const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(z3::udiv(lhsZ3->expr, rhsZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvSDiv(const ref<SolverHandle> &lhs,
                                    const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr / rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvAnd(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr & rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvOr(const ref<SolverHandle> &lhs,
                                  const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr & rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvXor(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr ^ rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvNot(const ref<SolverHandle> &arg) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  SolverHandle *handle = new Z3SolverHandle(~argZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvSle(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(z3::sle(lhsZ3->expr, rhsZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvSlt(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(z3::slt(lhsZ3->expr, rhsZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvUle(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(z3::ule(lhsZ3->expr, rhsZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvUlt(const ref<SolverHandle> &lhs,
                                   const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(z3::ult(lhsZ3->expr, rhsZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvZExt(const ref<SolverHandle> &arg,
                                    const ref<SolverHandle> &width) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  const ref<Z3SolverHandle> widthZ3 = cast<Z3SolverHandle>(width);
  SolverHandle *handle =
      new Z3SolverHandle(z3::zext(argZ3->expr, widthZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvSExt(const ref<SolverHandle> &arg,
                                    const ref<SolverHandle> &width) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  const ref<Z3SolverHandle> widthZ3 = cast<Z3SolverHandle>(width);
  SolverHandle *handle =
      new Z3SolverHandle(z3::sext(argZ3->expr, widthZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvShl(const ref<SolverHandle> &arg,
                                   const ref<SolverHandle> &width) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  const ref<Z3SolverHandle> widthZ3 = cast<Z3SolverHandle>(width);
  SolverHandle *handle =
      new Z3SolverHandle(z3::shl(argZ3->expr, widthZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvLShr(const ref<SolverHandle> &arg,
                                    const ref<SolverHandle> &width) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  const ref<Z3SolverHandle> widthZ3 = cast<Z3SolverHandle>(width);
  SolverHandle *handle =
      new Z3SolverHandle(z3::lshr(argZ3->expr, widthZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvAShr(const ref<SolverHandle> &arg,
                                    const ref<SolverHandle> &width) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  const ref<Z3SolverHandle> widthZ3 = cast<Z3SolverHandle>(width);
  SolverHandle *handle =
      new Z3SolverHandle(z3::ashr(argZ3->expr, widthZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvExtract(const ref<SolverHandle> &expr,
                                       const ref<SolverHandle> &off,
                                       const ref<SolverHandle> &len) {
  const ref<Z3SolverHandle> exprZ3 = cast<Z3SolverHandle>(expr);
  const ref<Z3SolverHandle> offZ3 = cast<Z3SolverHandle>(off);
  const ref<Z3SolverHandle> lenZ3 = cast<Z3SolverHandle>(len);
  SolverHandle *handle =
      new Z3SolverHandle(exprZ3->expr.extract(offZ3->expr, lenZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::bvConcat(const ref<SolverHandle> &lhs,
                                      const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle =
      new Z3SolverHandle(z3::concat(lhsZ3->expr, rhsZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::propAnd(const ref<SolverHandle> &lhs,
                                     const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr && rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::propOr(const ref<SolverHandle> &lhs,
                                    const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr || rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::propXor(const ref<SolverHandle> &lhs,
                                     const ref<SolverHandle> &rhs) {
  const ref<Z3SolverHandle> lhsZ3 = cast<Z3SolverHandle>(lhs);
  const ref<Z3SolverHandle> rhsZ3 = cast<Z3SolverHandle>(rhs);
  SolverHandle *handle = new Z3SolverHandle(lhsZ3->expr ^ rhsZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::propNot(const ref<SolverHandle> &arg) {
  const ref<Z3SolverHandle> argZ3 = cast<Z3SolverHandle>(arg);
  SolverHandle *handle = new Z3SolverHandle(!argZ3->expr);
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::propIte(const ref<SolverHandle> &cond,
                                     const ref<SolverHandle> &onTrue,
                                     const ref<SolverHandle> &onFalse) {
  const ref<Z3SolverHandle> condZ3 = cast<Z3SolverHandle>(cond);
  const ref<Z3SolverHandle> onTrueZ3 = cast<Z3SolverHandle>(onTrue);
  const ref<Z3SolverHandle> onFalseZ3 = cast<Z3SolverHandle>(onFalse);
  SolverHandle *handle = new Z3SolverHandle(
      z3::ite(condZ3->expr, onTrueZ3->expr, onFalseZ3->expr));
  return ref<SolverHandle>(handle);
}

ref<SolverHandle> Z3Adapter::array(const std::string &name,
                                   const ref<SortHandle> &sort) {
  const ref<Z3SortSolverHandle> sortZ3 = cast<Z3SortSolverHandle>(sort);
  return new Z3SolverHandle(ctx.constant(name.c_str(), sortZ3->sort));
}

ref<SolverHandle> Z3Adapter::read(const ref<SolverHandle> &array,
                                  const ref<SolverHandle> &index) {
  const ref<Z3SolverHandle> arrayZ3 = cast<Z3SolverHandle>(array);
  const ref<Z3SolverHandle> indexZ3 = cast<Z3SolverHandle>(index);
  return new Z3SolverHandle(z3::select(arrayZ3->expr, indexZ3->expr));
}

ref<SolverHandle> Z3Adapter::write(const ref<SolverHandle> &array,
                                   const ref<SolverHandle> &index,
                                   const ref<SolverHandle> &value) {
  const ref<Z3SolverHandle> arrayZ3 = cast<Z3SolverHandle>(array);
  const ref<Z3SolverHandle> indexZ3 = cast<Z3SolverHandle>(index);
  const ref<Z3SolverHandle> valueZ3 = cast<Z3SolverHandle>(value);
  return new Z3SolverHandle(
      z3::store(arrayZ3->expr, indexZ3->expr, valueZ3->expr));
}
