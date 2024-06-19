#ifndef ARRAYS_H
#define ARRAYS_H

#include "Propositional.h"
#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "TheoryHandle.h"
#include "klee/Expr/Expr.h"

#include <vector>

namespace klee {

/* Arrays theory */
template <typename DS, typename RS>
struct Arrays : public SolverTheory<Arrays<DS, RS>> {
private:
  ref<TheoryHandle<Arrays<DS, RS>>> array(const ref<ReadExpr> &readExpr) {
    const Array *array = readExpr->updates.root;

    // Create bitvector sorts
    ref<SortHandle> domainSort =
        DS(this->solverAdapter).sort(array->getDomain());
    ref<SortHandle> rangeSort = RS(this->solverAdapter).sort(array->getRange());
    ref<SortHandle> arraySort =
        this->solverAdapter->arraySort(domainSort, rangeSort);

    ref<TheoryHandle<Arrays<DS, RS>>> arrayHandle =
        new TheoryHandle<Arrays<DS, RS>>(
            this->solverAdapter->array(array->getName(), arraySort), readExpr);

    if (ref<ConstantSource> constantSource =
            dyn_cast<ConstantSource>(array->source)) {
      for (std::size_t i = 0; i < constantSource->constantValues.size(); ++i) {
        auto domainHandle = DS(this->solverAdapter)
                                .translate(ConstantExpr::create(i, sizeof(i)));
        auto valueHandle = RS(this->solverAdapter)
                               .translate(constantSource->constantValues[i]);
        auto eqHandle =
            Propositional(this->solverAdapter)
                .translate(EqExpr::create(ConstantExpr::create(i, sizeof(i)),
                                          constantSource->constantValues[i]));
        // TODO: put in the constraint set
      }
    }

    for (const ref<UpdateNode> &node : readExpr->updates) {
      auto domainHandle = DS(this->solverAdapter).translate(node->index);
      auto valueHandle = RS(this->solverAdapter).translate(node->value);
      arrayHandle = new decltype(arrayHandle)(
          write(arrayHandle, domainHandle, valueHandle), readExpr);
    }

    return arrayHandle;
  }

public:
  template <typename... Args>
  ref<TheoryHandle<Arrays<DS, RS>>> translate(const ref<Expr> &expr,
                                              Args &&...args) {
    switch (expr->getKind()) {
    case Expr::Kind::Read: {
      return read(expr, array(cast<ReadExpr>(expr)), args...);
    }
    default: {
      return nullptr;
    }
    }
  }

  Arrays(const ref<SolverAdapter> &solverAdapter)
      : SolverTheory<Arrays<DS, RS>>(SolverTheory<Arrays<DS, RS>>::ARRAYS,
                                     solverAdapter) {}

  std::string toString() const override { return "Arrays"; }

  ref<SortHandle> sort(const ref<SortHandle> &domainSort,
                       const ref<SortHandle> &rangeSort) {
    return this->solverAdapter->arraySort(domainSort, rangeSort);
  }

  ref<TheoryHandle<RS>> read(const ref<Expr> &expr,
                             const ref<TheoryHandle<Arrays<DS, RS>>> &array,
                             const ref<TheoryHandle<DS>> &index) {
    return new TheoryHandle<Arrays<DS, RS>>(
        this->solverAdapter->read(array, index), expr);
  }

  ref<TheoryHandle<Arrays<DS, RS>>>
  write(const ref<Expr> &expr, const ref<TheoryHandle<Arrays<DS, RS>>> &array,
        const ref<TheoryHandle<DS>> &index,
        const ref<TheoryHandle<RS>> &value) {
    return new TheoryHandle<Arrays<DS, RS>>(
        this->solverAdapter->write(array, index, value));
  }

  template <typename T> static bool classof(const SolverTheory<T> *th) {
    return th->getSort() == SolverTheory<Arrays<DS, RS>>::ARRAYS;
  }
};

} // namespace klee

#endif
