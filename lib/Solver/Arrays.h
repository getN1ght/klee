#ifndef ARRAYS_H
#define ARRAYS_H

#include "SolverTheory.h"

/* Arrays theory */
template <typename RT, typename DT> struct Arrays : public SolverTheory {
protected:
  ref<ExprHandle> translate(const ref<Expr> &, const ArgumentsList &) override {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      return read(args[0], args[1]);
    }
    default: {
      return nullptr;
    }
    }
  }

public:
  ref<ExprHandle> sort() override {
    // RT *rangeSort = new RT();
    // DT *domainSort = new DT();
    // return solverAdapter->arraySort(rangeSort, domainSort);
  }

  ref<ExprHandle> read(const ref<ExprHandle> &array,
                       const ref<ExprHandle> &index) {
    return solverAdapter->read(arr, idx);
  }

  ref<ExprHandle> write(const ref<ExprHandle> &array,
                        const ref<ExprHandle> &index,
                        const ref<ExprHandle> &value) {
    return solverAdapter->write(arr, idx, value);
  }
};

#endif
