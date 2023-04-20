#include "Z3TheoryArray.h"

#include "Z3Builder.h"
#include "z3.h"

using namespace klee;

Z3ASTHandle Z3TheoryArray::buildArray(const char *name, unsigned int indexWidth,
                                      unsigned int valueWidth) {
  Z3SortHandle domainSort; // = getBvSort(indexWidth);
  Z3SortHandle rangeSort; // = getBvSort(valueWidth);
  Z3SortHandle t = arraySort(domainSort, rangeSort);
  Z3_symbol s = Z3_mk_string_symbol(ctx, const_cast<char *>(name));
  return Z3ASTHandle(Z3_mk_const(ctx, s, t), ctx);
}

Z3ASTHandle Z3TheoryArray::buildConstantArray(unsigned int indexWidth, const Z3ASTHandle &defaultValue) {
  Z3SortHandle domainSort;
  return Z3ASTHandle(Z3_mk_const_array(ctx, domainSort, defaultValue), ctx);
}

Z3SortHandle Z3TheoryArray::arraySort(const Z3SortHandle &domainSort,
                                      const Z3SortHandle &rangeSort) {
  Z3_sort sort = Z3_mk_array_sort(ctx, domainSort, rangeSort);
  // TODO: hash received sort.
  return Z3SortHandle(sort, ctx);
}

Z3ASTHandle Z3TheoryArray::getInitialArray(const Array *root) {
  assert(root);
  Z3ASTHandle arrayExpr;
  if (arrHash.lookupArrayExpr(root, arrayExpr)) {
    return arrayExpr;
  }

  // Unique arrays by name, so we make sure the name is unique by
  // using the size of the array hash as a counter.
  std::string unique_id = llvm::utostr(arrHash._array_hash.size());
  std::string unique_name = root->name + unique_id;
  if (ref<ConstantWithSymbolicSizeSource> constantWithSymbolicSizeSource =
          dyn_cast<ConstantWithSymbolicSizeSource>(root->source)) {
    arrayExpr = buildConstantArray(
        root->getDomain(), constantWithSymbolicSizeSource->defaultValue);
  } else {
    arrayExpr =
        buildArray(unique_name.c_str(), root->getDomain(), root->getRange());
  }

  // Create optimizing assertions on given array.
  if (root->isConstantArray() && constant_array_assertions.count(root) == 0) {
    std::vector<Z3ASTHandle> array_assertions;
    for (unsigned i = 0, e = root->constantValues.size(); i != e; ++i) {
      // construct(= (select i root) root->value[i]) to be asserted in
      // Z3Solver.cpp
      Z3ASTHandle array_value =
          construct(root->constantValues[i]);
      array_assertions.push_back(
          eqExpr(readExpr(arrayExpr, constantExpr(root->getDomain(), i)),
                 array_value));
    }
    constantArrayAssertions[root] = std::move(array_assertions);
  }

  arrHash.hashArrayExpr(root, arrayExpr);

  return arrayExpr;
}

Z3ASTHandle Z3TheoryArray::writeExpr(Z3ASTHandle array, Z3ASTHandle index,
                                 Z3ASTHandle value) {
  return Z3ASTHandle(Z3_mk_store(ctx, array, index, value), ctx);
}

Z3ASTHandle Z3TheoryArray::readExpr(Z3ASTHandle array, Z3ASTHandle index) {
  return Z3ASTHandle(Z3_mk_select(ctx, array, index), ctx);
}
