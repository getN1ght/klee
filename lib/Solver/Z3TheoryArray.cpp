#include "Z3TheoryArray.h"

#include "Z3Builder.h"
#include "z3.h"

using namespace klee;

Z3ASTHandle Z3TheoryArray::buildArray(const char *name, unsigned int indexWidth,
                                      unsigned int valueWidth) {
  return klee::Z3ASTHandle();
}

Z3SortHandle Z3TheoryArray::arraySort(Z3SortHandle domainSort,
                                      Z3SortHandle rangeSort) {
  Z3_sort sort = Z3_mk_array_sort(ctx, domainSort, rangeSort);

  return klee::Z3SortHandle();
}

Z3ASTHandle Z3TheoryArray::getInitialArray(const Array *array) {
  return klee::Z3ASTHandle();
}


Z3ASTHandle Z3TheoryArray::readExpr(Z3ASTHandle array, Z3ASTHandle index) {

}

Z3ASTHandle Z3TheoryArray::writeExpr(Z3ASTHandle array, Z3ASTHandle index,
                      Z3ASTHandle value) {

}
