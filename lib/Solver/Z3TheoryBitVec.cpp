#include "Z3TheoryBitVec.h"

#include "Z3Builder.h"

using namespace klee;

Z3SortHandle Z3TheoryBitVec::bvSort(unsigned int width) {
  return Z3SortHandle(Z3_mk_bv_sort(ctx, width), ctx);
}

Z3ASTHandle Z3TheoryBitVec::constantExpr(unsigned int width, uint64_t value) {
  return Z3ASTHandle(Z3_mk_unsigned_int64(ctx, value, bvSort(width)), ctx);
}

Z3ASTHandle Z3TheoryBitVec::addExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvadd(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::subExpr(const Z3ASTHandle &lhs,
                            const Z3ASTHandle &rhs){
  return Z3ASTHandle(Z3_mk_bvsub(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::mulExpr(const Z3ASTHandle &lhs,
                            const Z3ASTHandle &rhs){
  return Z3ASTHandle(Z3_mk_bvmul(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::sdivExpr(const Z3ASTHandle &lhs,
                             const Z3ASTHandle &rhs){
  return Z3ASTHandle(Z3_mk_bvsdiv(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::udivExpr(const Z3ASTHandle &lhs,
                             const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvudiv(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::notExpr(const Z3ASTHandle &expr) {
  return Z3ASTHandle(Z3_mk_not(ctx, expr), ctx);
}

Z3ASTHandle Z3TheoryBitVec::andExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvand(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::orExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvor(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::xorExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvxor(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::eqExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_eq(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::zextExpr(const Z3ASTHandle &expr, uint64_t width) {
  return Z3ASTHandle(Z3_mk_zero_ext(ctx, width, expr), ctx);
}

Z3ASTHandle Z3TheoryBitVec::sextExpr(const Z3ASTHandle &expr, uint64_t width){
  return Z3ASTHandle(Z3_mk_sign_ext(ctx, width, expr), ctx);
}

Z3ASTHandle Z3TheoryBitVec::uleExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvule(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::ultExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvult(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::ugeExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvuge(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::ugtExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvugt(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::sleExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvsle(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::sltExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvslt(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::sgeExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvsge(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::sgtExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return Z3ASTHandle(Z3_mk_bvsgt(ctx, lhs, rhs), ctx);
}

Z3ASTHandle Z3TheoryBitVec::iteExpr(const Z3ASTHandle &condition,
                                    const Z3ASTHandle &whenTrue,
                                    const Z3ASTHandle &whenFalse) {
  return Z3ASTHandle(Z3_mk_ite(ctx, condition, whenTrue, whenFalse), ctx);
}
