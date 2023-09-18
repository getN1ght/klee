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
  ref<TheoryHandle> array(const ref<ReadExpr> &readExpr) {
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
  ref<TheoryHandle> translate(const ref<Expr> &expr,
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

  ref<TheoryHandle> sort(const ref<TheoryHandle> &domainSort,
                         const ref<TheoryHandle> &rangeSort) {
    return apply(std::bind(std::mem_fn(&SolverAdapter::array), solverAdapter,
                           std::placeholders::_1, std::placeholders::_2), domainSort, rangeSort);
  }

  ref<TheoryHandle> read(const ref<TheoryHandle> &array,
                         const ref<TheoryHandle> &index) {
    return apply(std::bind(std::mem_fn(&SolverAdapter::read), solverAdapter,
                           std::placeholders::_1, std::placeholders::_2), array, index);
  }

  ref<TheoryHandle> write(const ref<TheoryHandle> &array,
                          const ref<TheoryHandle> &index,
                          const ref<TheoryHandle> &value) {
    
    return solverAdapter->write(array, index, value);
  }
};

} // namespace klee

#endif
