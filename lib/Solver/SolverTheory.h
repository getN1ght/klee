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

public:
  enum Sort { ARRAYS, BOOL, BV, FPBV, LIA };
  const Sort theorySort;

  // typedef ref<TheoryHandle> (SolverTheory::*cast_function_t)(
  //     const ref<TheoryHandle> &);

private:
  template <typename... Args> bool isAllValid(const Args &&...args) {
    return ([&](const auto &arg) -> bool { return arg.has_value(); }(args) &&
                                        ...);
  }

  // std::unordered_map<SolverTheory::Sort, cast_function_t> castMapping;
protected:
  ref<SolverAdapter> solverAdapter;

  // FIXME: do casts in another way
  // protected:
  // template <typename D, typename R>
  // virtual ref<TheoryHandle<Arrays<D, R>>>
  // castToArray(const ref<TheoryHandle> &arg);

  // virtual ref<TheoryHandle<BV>> castToBV(const ref<TheoryHandle<>> &arg);
  // virtual ref<TheoryHandle<Propositional>>
  // castToBool(const ref<TheoryHandle> &arg);
  // virtual ref<TheoryHandle<FPBV>> castToFPBV(const ref<TheoryHandle> &arg);
  // virtual ref<TheoryHandle<LIA>> castToLIA(const ref<TheoryHandle> &arg);

public:
  SolverTheory(Sort, const ref<SolverAdapter> &);

  virtual std::string toString() const = 0;

  int compare(const SolverTheory &rhs) const {
    if (getSort() < rhs.getSort()) {
      return -1;
    }
    return getSort() > rhs.getSort();
  }

  /*
   * Translates the given expression into a theory expression.
   * Note: using ``Curiously recurring template pattern trick''.
   * Allows to make an analogue of templated virtual function.
   */
  template <typename RT, typename... Args>
  ref<TheoryHandle<RT>> translate(const ref<Expr> &expr, const Args &&...args) {
    return static_cast<D *>(this)->translate(expr, args...);
  }

  // ref<TheoryHandle> castTo(Sort sort, const ref<TheoryHandle> &arg);

  Sort getSort() const { return theorySort; }

  virtual ~SolverTheory() = default;
};

} // namespace klee

#endif
