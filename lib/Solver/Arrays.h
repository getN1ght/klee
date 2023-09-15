#ifndef ARRAYS_H
#define ARRAYS_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/Expr/Expr.h"

#include <map>
#include <vector>

namespace klee {

/* Arrays theory */
template <typename DS, typename RS> struct Arrays : public SolverTheory {
  friend class SolverTheory;

private:
  ref<TheoryResponse> array(const ref<ReadExpr> &readExpr) {
    const Array *array = readExpr->updates.root;

    // Create bitvector sorts
    ref<ExprHandle> domainSort = DS(solverAdapter).sort(array->getDomain());
    ref<ExprHandle> rangeSort = RS(solverAdapter).sort(array->getRange());

    ref<ExprHandle> result = solverAdapter->array(domainSort, rangeSort);

    std::vector<ref<Expr>> required;
    ref<ConstantSource> constantSource =
        dyn_cast<ConstantSource>(array->source);

    uint64_t exprToBuildInUpdateList = readExpr->updates.getSize() * 2;

    if (constantSource) {
      required.reserve(constantSource->constantValues.size() +
                       exprToBuildInUpdateList);
      for (ref<Expr> constantWriteExpr : constantSource->constantValues) {
        required.push_back(constantWriteExpr);
      }
    } else {
      required.reserve(exprToBuildInUpdateList);
    }

    // TODO: add iterator in UpdateList
    for (ref<UpdateNode> node = readExpr->updates.head; node != nullptr;
         node = node->next) {
      required.push_back(node->index);
      required.push_back(node->value);
    }

    IncompleteResponse::completer_t completer =
        [*this, result, readExpr,
         constantSource](const ExprHashMap<ref<ExprHandle>> &map) mutable
        -> ref<ExprHandle> {
      /*
       * TODO: should be located here, or somewhere else?
       *
       * Optimization on values on constant indices.
       * Forms expression in form of
       * - ReadExpr Idx Array == ConstantExpr
       */
      if (constantSource) {
        const std::vector<ref<ConstantExpr>> &constantValues =
            constantSource->constantValues;
        for (unsigned idx = 0; idx < constantValues.size(); ++idx) {
          ref<ExprHandle> constantAssertion = map.at(constantValues.at(idx));
        }
      }

      ref<UpdateNode> node = readExpr->updates.head;
      while (!node.isNull()) {
        result = write(result, map.at(node->index), map.at(node->value));
        node = node->next;
      }
      return result;
    };

    return new IncompleteResponse(completer, required);
  }

protected:
  ref<TheoryResponse> translate(const ref<Expr> &expr,
                                const ExprHandleList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      return apply(&Arrays::read, array(cast<ReadExpr>(expr)), args[1]);
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
