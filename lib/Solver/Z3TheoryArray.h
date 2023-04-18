#include "Z3Theory.h"

#include "Z3Builder.h"

#ifndef KLEE_Z3_THEORY_ARRAY_H
#define KLEE_Z3_THEORY_ARRAY_H

namespace klee {

class Array;

struct Z3TheoryArray : public Z3Theory {
private:
  Z3ArrayExprHash arrHash;

  Z3ASTHandle buildArray(const char *name, unsigned indexWidth,
                  unsigned valueWidth);
  Z3SortHandle arraySort(Z3SortHandle domainSort, Z3SortHandle rangeSort);

  Z3ASTHandle getInitialArray(const Array *array);


public:
  Z3ASTHandle readExpr(Z3ASTHandle array, Z3ASTHandle index) override;
  Z3ASTHandle writeExpr(Z3ASTHandle array, Z3ASTHandle index,
                                Z3ASTHandle value) override;
};
}

#endif // KLEE_Z3_THEORY_ARRAY_H
