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
    ref<SortHandle> domainSort = DS(solverAdapter).sort(array->getDomain());
    ref<SortHandle> rangeSort = RS(solverAdapter).sort(array->getRange());
    ref<SortHandle> arraySort = solverAdapter->arraySort(domainSort, rangeSort);

    ref<TheoryHandle> arrayHandle = new CompleteTheoryHandle(
        solverAdapter->array(array->getName(), arraySort), this);

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

    for (const ref<UpdateNode> &node : readExpr->updates) {
      required.push_back(node->index);
      required.push_back(node->value);
    }

    if (required.size() == 0) {
      return arrayHandle;
    } 

    IncompleteResponse::completer_t completer =
        [*this, arrayHandle, readExpr, constantSource](
            const IncompleteResponse::TheoryHandleProvider &map) mutable
        -> ref<TheoryHandle> {
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
          ref<TheoryHandle> constantAssertion = map.at(constantValues.at(idx));
        }
      }

      ref<UpdateNode> node = readExpr->updates.head;
      while (!node.isNull()) {
        arrayHandle =
            write(arrayHandle, map.at(node->index), map.at(node->value));
        node = node->next;
      }
      return arrayHandle;
    };

    return new IncompleteResponse(this, completer, required);
  }

protected:
  ref<TheoryHandle> translate(const ref<Expr> &expr,
                              const TheoryHandleList &args) override {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      return read(array(cast<ReadExpr>(expr)), args[0]);
    }
    default: {
      return new BrokenTheoryHandle(expr);
    }
    }
  }

public:
  Arrays(const ref<SolverAdapter> &solverAdapter)
      : SolverTheory(SolverTheory::ARRAYS, solverAdapter) {}

  std::string toString() const override {
    return "Arrays"; 
  }

  ref<SortHandle> sort(const ref<SortHandle> &domainSort,
                       const ref<SortHandle> &rangeSort) {
    return solverAdapter->arraySort(domainSort, rangeSort);
  }

  ref<TheoryHandle> read(const ref<TheoryHandle> &array,
                         const ref<TheoryHandle> &index) {
    ref<TheoryHandle> readHandle =
        apply(std::bind(&SolverAdapter::read, solverAdapter,
                        std::placeholders::_1, std::placeholders::_2),
              array, index);
    // FIXME: we should NOT create a new instance of theory
    readHandle->parent = new RS(solverAdapter);
    return readHandle;
  }

  ref<TheoryHandle> write(const ref<TheoryHandle> &array,
                          const ref<TheoryHandle> &index,
                          const ref<TheoryHandle> &value) {
    return apply(std::bind(&SolverAdapter::write, solverAdapter,
                           std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3),
                 array, index, value);
  } 

  static bool classof(const SolverTheory *th) {
    return th->getSort() == Sort::ARRAYS;
  }
};

} // namespace klee

#endif
