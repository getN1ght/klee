#include "Z3LIABuilder.h"

#include "Z3Builder.h"

#include "z3.h"

using namespace klee;

Z3SortHandle Z3LIABuilder::getIntSort() {
  return Z3SortHandle(Z3_mk_int_sort(ctx), ctx);
}

Z3ASTLIAHandle Z3LIABuilder::buildArray(const char *name, unsigned indexWidth,
                                        unsigned valueWidth) {
  Z3SortHandle domainSort = getIntSort();
  Z3SortHandle rangeSort = getIntSort();

  Z3SortHandle t = getArraySort(domainSort, rangeSort);
  Z3_symbol s = Z3_mk_string_symbol(ctx, const_cast<char *>(name));
  return Z3ASTLIAHandle(Z3_mk_const(ctx, s, t), ctx);
}

Z3ASTLIAHandle Z3LIABuilder::uge(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  return Z3ASTLIAHandle(Z3_mk_ge(ctx, lhs, rhs), ctx);
}
Z3ASTLIAHandle Z3LIABuilder::ule(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  return Z3ASTLIAHandle(Z3_mk_le(ctx, lhs, rhs), ctx);
}

Z3ASTLIAHandle Z3LIABuilder::ult(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  return Z3ASTLIAHandle(Z3_mk_lt(ctx, lhs, rhs), ctx);
}

Z3ASTLIAHandle Z3LIABuilder::ugt(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  return Z3ASTLIAHandle(Z3_mk_gt(ctx, lhs, rhs), ctx);
}

Z3ASTLIAHandle Z3LIABuilder::sge(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}
Z3ASTLIAHandle Z3LIABuilder::sle(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}
Z3ASTLIAHandle Z3LIABuilder::slt(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}
Z3ASTLIAHandle Z3LIABuilder::sgt(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}

Z3ASTLIAHandle Z3LIABuilder::eq(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  return Z3ASTLIAHandle(Z3_mk_eq(ctx, lhs, rhs), ctx);
}

Z3ASTLIAHandle Z3LIABuilder::add(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  Z3_ast nodeList[2] = {lhs, rhs};

  Z3_ast addNode = Z3_mk_add(ctx, 2, nodeList);
  Z3ASTLIAHandle res(addNode, ctx);

  return res;
}

Z3ASTLIAHandle Z3LIABuilder::sub(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  Z3_ast nodeList[2] = {lhs, rhs};

  Z3_ast addNode = Z3_mk_sub(ctx, 2, nodeList);
  Z3ASTLIAHandle res(addNode, ctx);

  return res;
}

Z3ASTLIAHandle Z3LIABuilder::mult(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  Z3_ast nodeList[2] = {lhs, rhs};

  Z3_ast addNode = Z3_mk_mul(ctx, 2, nodeList);
  Z3ASTLIAHandle res(addNode, ctx);

  return res;
}

Z3ASTLIAHandle Z3LIABuilder::div(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {
  Z3_ast addNode = Z3_mk_div(ctx, lhs, rhs);
  Z3ASTLIAHandle res(addNode, ctx);

  return res;
}

Z3ASTLIAHandle Z3LIABuilder::lshr(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}

Z3ASTLIAHandle Z3LIABuilder::ashr(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}

Z3ASTLIAHandle Z3LIABuilder::shl(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}

Z3ASTLIAHandle Z3LIABuilder::zext(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}
Z3ASTLIAHandle Z3LIABuilder::sext(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs) {}
