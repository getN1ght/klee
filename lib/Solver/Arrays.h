#ifndef ARRAYS_H
#define ARRAYS_H

#include "SolverTheory.h"
#include "SolverAdapter.h"

#include "klee/Expr/Expr.h"

#include <map>

namespace klee {

/* Arrays theory */
struct Arrays : public SolverTheory {
private:
  ref<ExprHandle> array(const ref<ReadExpr> &readExpr) {
    /*
     * Optimization on values on constant indices.
     * Forms expression in form of 
     * - ReadExpr Idx Array == ConstantExpr 
     */
    const Array *array = readExpr->updates.root;
    if (ref<ConstantSource> constantSource =
            dyn_cast<ConstantSource>(array->source)) {
      const std::vector<ref<ConstantExpr>> &constantValues =
          constantSource->constantValues;
      for (unsigned idx = 0; idx < constantValues.size(); ++idx) {
        constantValues[idx];
        // TODO:
      }
    }

    // FIXME: initialize
    ref<ExprHandle> result = nullptr;

    ref<UpdateNode> node = readExpr->updates.head;
    while (node != nullptr) {
      result = write(result, node->index, node->value);
      node = node->next;
    }
    return result;
  }

protected:
  ref<ExprHandle> translate(const ref<Expr> &expr, const ArgumentsList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      ref<ReadExpr> readExpr = cast<ReadExpr>(expr);

      return read(args[0], args[1]);
    }
    default: {
      return nullptr;
    }
    }
  }

public:
  // ref<ExprHandle> sort(const ref<ExprHandle> &domainSort,
  //                      const ref<ExprHandle> &rangeSort) override {
  //   return solverAdapter->arraySort(domainSort, rangeSort);
  // }

  ref<ExprHandle> read(const ref<ExprHandle> &array,
                       const ref<ExprHandle> &index) {
    return solverAdapter->read(array, index);
  }

  ref<ExprHandle> write(const ref<ExprHandle> &array,
                        const ref<ExprHandle> &index,
                        const ref<ExprHandle> &value) {
    return solverAdapter->write(array, index, value);
  }
};

} // namespace klee

#endif
