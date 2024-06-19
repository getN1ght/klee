#ifndef _SOLVERTHEORY_H
#define _SOLVERTHEORY_H

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "TheoryHandle.h"

namespace klee {

/*
 * TODO: check
 * https://stackoverflow.com/questions/26129933/bind-to-function-with-an-unknown-number-of-arguments-in-c.
 * Seems we may auto generate the correct number of arguments.
 */

class SolverAdapter;
template <typename> class ExprHashMap;

/*
 * Base class for handles for all builders.
 * Maintains a inner representation for solver
 * and type of expression inside.
 */
template <typename D> class SolverTheory {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  enum Sort { ARRAYS, BOOL, BV, FPBV, LIA };
  const Sort theorySort;

private:
  template <typename... Args> bool isAllValid(const Args &&...args) {
    return ([&](const auto &arg) -> bool { return arg.has_value(); }(args) &&
                                        ...);
  }

protected:
  ref<SolverAdapter> solverAdapter;

public:
  SolverTheory(Sort sort, ref<SolverAdapter> solverAdapter)
      : theorySort(sort), solverAdapter(solverAdapter) {}

  virtual std::string toString() const = 0;

  int compare(const SolverTheory &rhs) const {
    if (getSort() < rhs.getSort()) {
      return -1;
    }
    return getSort() > rhs.getSort();
  }

  /*
   * Translates the given expression into a theory expression.
   * TODO: Useless comment below
   Note: using ``Curiously recurring template pattern trick''.
   * Allows to make an analogue of templated virtual function.
   */
  template <typename RT, typename... Args>
  ref<TheoryHandle<RT>> translate(const ref<Expr> &expr, const Args &&...args) {
    return static_cast<D *>(this)->translate(expr, args...);
  }

  Sort getSort() const { return theorySort; }

  virtual ~SolverTheory() = default;
};

} // namespace klee

#endif
