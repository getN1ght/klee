#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"
#include "klee/util/EDM.h"

#include "llvm/Support/Casting.h"

#include "TheoryHandle.h"

#include <type_traits>
#include <unordered_map>

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
template <typename D> struct SolverTheory {
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
    return (![&](auto arg) -> bool {
      return !isa<BrokenTheoryHandle>(arg);
    }(args) && ...);
  }

  template <typename... Args>
  std::vector<ref<Expr>> collectRequired(const Args &&...args) {
    std::vector<ref<Expr>> required;
    (
        [&](auto arg) {
          if (ref<IncompleteResponse> incompleteResponse =
                  dyn_cast<IncompleteResponse>(arg)) {
            required.insert(required.end(), incompleteResponse->toBuild.begin(),
                            incompleteResponse->toBuild.end());
          }
        }(args),
        ...);
    return required;
  }

public:
  /*
   * Applies the given function to the list of
   * args if args satisfy the signature. Also
   * checks the completeness of all arguments,
   * and if argument is incomplete, then returns
   * incomplete handle with functor able to complete
   * entire handle.
   */
  template <typename Functor, typename RT, typename... Args>
  ref<TheoryHandle<RT>> apply(const Functor &functor, Args &&...args) {
    /*
     * If at least one argument is broken, then we
     * can not build handle for that expression. Hence,
     * we return a broken handle.
     */
    if (isAllValid(args)) {
      /// FIXME: report with correct expression.
      return new BrokenTheoryHandle(nullptr);
    }

    const std::vector<ref<Expr>> toBuild = collectRequired(args...);

    /*
     * Here we prepared array of expressions ``toBuild'', which are required
     * to complete the handle. If this array is empty, then we may complete the
     * handle right now. Otherwise, we return a completer, which require
     * completed handles for all expressions from that list.
     */
    IncompleteResponse::completer_t completer =
        [functor,
         args...](const IncompleteResponse::TheoryHandleProvider &provider)
        -> ref<SolverHandle> {
      
      auto unwrap =
          [&provider](auto arg) -> ref<SolverHandle> {

        /* Get the type of theory of received handle */
        using theory_t = typename std::decay_t<decltype(arg)>::theory_t;

        if (ref<IncompleteResponse<theory_t>> incompleteTheoryHandle =
                dyn_cast<IncompleteResponse<theory_t>>(arg)) {
          arg = incompleteTheoryHandle->complete(provider);
        }
        if (ref<CompleteTheoryHandle<theory_t>> completeTheoryHandle =
                dyn_cast<CompleteTheoryHandle<theory_t>>(arg)) {
          return completeTheoryHandle->expr();
        }
        std::abort();
        return nullptr;
      };

      /*
       * Check if functor applicable to given args and returns appropriate type.
       */
      if constexpr (std::is_invocable_v<functor, decltype(unwrap(args))...>) {
        return functor(unwrap(args)...);
      } else {
        /* FIXME: Report an error in given expression */
        return new BrokenTheoryHandle(nullptr);
      }

    };

    if (toBuild.empty()) {
      return new CompleteTheoryHandle<RT>(
          completer(IncompleteResponse::TheoryHandleProvider()), expr);
    } else {
      return new IncompleteResponse(completer, toBuild);
    }
  }

  /* FIXME: figure out better style */
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
