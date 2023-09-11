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
  /* TODO: should theory know anything about builders? */
  ref<SolverBuilder> parentBuilder = nullptr;

  // TODO: This code should be located in SolverBuilder.h  
  ref<ExprHandle> array(const ref<ReadExpr> &readExpr) {
    ref<ExprHandle> result = nullptr;

    ref<UpdateNode> node = readExpr->updates.head;
    while (node != nullptr) {
      ref<ExprHandle> indexHandle = parentBuilder->build(node->index);
      ref<ExprHandle> valueHandle = parentBuilder->build(node->value);

      result = write(result, indexHandle, valueHandle);
      node = node->next;
    }

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
        ref<ExprHandle> constantAssertion =
            parentBuilder->build(constantValues.at(idx));

        result->pushSideConstraint(constantAssertion);
      }
    }

    return result;
  }

protected:
  ref<ExprHandle> translate(const ref<Expr> &expr,
                            const ExprHandleList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      ref<ReadExpr> readExpr = cast<ReadExpr>(expr);
      return read(array(readExpr), args[0]);
    }
    default: {
      return nullptr;
    }
    }
  }

public:
  Arrays(const ref<SolverAdapter> &solverAdapter)
      : SolverTheory(solverAdapter) {}

  ref<ExprHandle> sort(const ref<ExprHandle> &domainSort,
                       const ref<ExprHandle> &rangeSort) {
    return solverAdapter->array(domainSort, rangeSort);
  }

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
