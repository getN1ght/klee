#ifndef ARRAYS_H
#define ARRAYS_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "klee/Expr/Expr.h"

#include <map>
#include <vector>

namespace klee {

/* Arrays theory */
template <typename DS, typename RS>
struct Arrays : public SolverTheory<Arrays<DS, RS>> {
private:
  ref<TheoryHandle<Arrays<DS, RS>>> array(const ref<ReadExpr> &readExpr) {
    const Array *array = readExpr->updates.root;

    // Create bitvector sorts
    ref<SortHandle> domainSort = DS(solverAdapter).sort(array->getDomain());
    ref<SortHandle> rangeSort = RS(solverAdapter).sort(array->getRange());
    ref<SortHandle> arraySort = solverAdapter->arraySort(domainSort, rangeSort);

    ref<TheoryHandle<Arrays<DS, RS>>> arrayHandle =
        new CompleteTheoryHandle<Arrays<DS, RS>>(
            solverAdapter->array(array->getName(), arraySort), readExpr);

    std::vector<ref<Expr>> required;
    uint64_t exprToBuildInUpdateList = readExpr->updates.getSize() * 2;

    if (ref<ConstantSource> constantSource =
            dyn_cast<ConstantSource>(array->source)) {
      required.reserve(constantSource->constantValues.size() +
                       exprToBuildInUpdateList);
      for (ref<Expr> constantWriteExpr : constantSource->constantValues) {
        required.push_back(constantWriteExpr);
      }
    } else {
      required.reserve(exprToBuildInUpdateList);
    }

    for (const ref<UpdateNode> &node : readExpr->updates) {
      required.push_back(node->index);
      required.push_back(node->value);
    }

    if (required.size() == 0) {
      return arrayHandle;
    }

    IncompleteResponse::completer_t completer =
        [*this, arrayHandle,
         readExpr](const IncompleteResponse::TheoryHandleProvider &map) mutable
        -> ref<TheoryHandle<Arrays<DS, RS>>> {
      /*
       * TODO: should be located here, or somewhere else?
       *
       * Optimization on values on constant indices.
       * Forms expression in form of
       * - ReadExpr Idx Array == ConstantExpr
       */
      // if (ref<ConstantSource> constantSource =
      //   dyn_cast<ConstantSource>(readExpr->array->source)) {
      //   const std::vector<ref<ConstantExpr>> &constantValues =
      //       constantSource->constantValues;
      //   for (unsigned idx = 0; idx < constantValues.size(); ++idx) {
      //     ref<TheoryHandle> constantAssertion =
      //     map.at(constantValues.at(idx));
      //   }
      // }

      for (const ref<UpdateNode> &node : readExpr->updates) {
        arrayHandle =
            write(arrayHandle, map.at(node->index), map.at(node->value));
      }
      return arrayHandle;
    };

    return new IncompleteResponse<Arrays<DS, RS>>(this, completer, required);
  }

public:
  template <typename... Args>
  ref<TheoryHandle> translate(const ref<Expr> &expr, const Args... &&args) {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      return read(expr, array(cast<ReadExpr>(expr)), args...);
    }
    default: {
      return new BrokenTheoryHandle(expr);
    }
    }
  }

  Arrays(const ref<SolverAdapter> &solverAdapter)
      : SolverTheory(SolverTheory::ARRAYS, solverAdapter) {}

  std::string toString() const override { return "Arrays"; }

  ref<SortHandle> sort(const ref<SortHandle> &domainSort,
                       const ref<SortHandle> &rangeSort) {
    return solverAdapter->arraySort(domainSort, rangeSort);
  }

  ref<TheoryHandle<RS>> read(const ref<Expr> &expr,
                             const ref<TheoryHandle<Arrays<DS, RS>>> &array,
                             const ref<TheoryHandle<DS>> &index) {
    return apply(std::bind(&SolverAdapter::read, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2),
                 array, index);
  }

  ref<TheoryHandle<Arrays<DS, RS>>>
  write(const ref<TheoryHandle<Arrays<DS, RS>>> &array,
        const ref<TheoryHandle<DS>> &index,
        const ref<TheoryHandle<RS>> &value) {
    return apply(std::bind(&SolverAdapter::write, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3),
                 array, index, value);
  }

  template <typename T> static bool classof(const SolverTheory<T> *th) {
    return th->getSort() == Sort::ARRAYS;
  }
};

} // namespace klee

#endif
