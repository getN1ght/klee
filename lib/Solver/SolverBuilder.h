#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "SolverTheory.h"

#include "klee/ADT/BiMap.h"
#include "klee/ADT/Ref.h"

#include "klee/Expr/ExprHashMap.h"

#include "klee/util/EDM.h"

#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace klee {

class Expr;

template <typename... Theory> class SolverBuilder {

  template <typename... RawTheory> friend class RawSolverBuilder;
  
  typedef std::variant<ref<TheoryHandle<Theory...>>> TheoryHandle_t;
  typedef ExprHashMap<std::variant<ref<TheoryHandle<Theory...>>>> cache_t;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

private:
  /* Cache for already built expressions */
  cache_t cache;

  SolverBuilder() {
    static_assert(sizeof...(Args) != 0,
                  "no theories specified for the builder");
  }

  template <typename RT, typename AT>
  ref<TheoryHandle<RT>> buildWithTheory(const ref<SolverTheory<AT>> &theory,
                                        const ref<Expr> &expr) {}

  // ref<TheoryHandle> castToTheory(const ref<TheoryHandle> &arg,
  //                                SolverTheory::Sort sort);

public:
  /*
   * Translates KLEE's inner representation of expression
   * to the expression for the solver, specified in solverAdapter.
   */
  std::variant<ref<TheoryHandle<Theory>>...> build(const ref<Expr> &expr) {
    if (cache.count(expr)) {
      return cache.at(expr);
    }

    /*
     * Iterating over theories in order to find capable
     * of translating given expression.
     */
    (
        [&](auto &&theory) {
          auto exprHandle = buildWithTheory(theory, expr);
          using RT =
              typename std::decay_t<decltype(exprHandle.get())>::theory_t;

          /*
           * If handle is broken, then expression can not be built
           * in that theory. Try another one.
           */
          if (isa<BrokenTheoryHandle<RT>>(exprHandle)) {
            continue;
          }

          /*
           * If handle is incomplete, then we should subscribe
           * on it in order to cache it as soon as we will be able to construct
           * it.
           */
          if (ref<IncompleteResponse<RT>> incompleteExprHandle =
                  dyn_cast<IncompleteResponse<RT>>(exprHandle)) {
            // incompleteExprHandle->listen(this);
          }

          cache.emplace(expr, exprHandle);
          return exprHandle;
        }(new Theory(nullptr)),
        ...);

    /*
     * All theories can not be use to translate current expression
     * in solver's expression. Memoize that we can not build it
     * and return broken handle.
     */
    cache.emplace(expr, new BrokenTheoryHandle(expr));
    return cache.at(expr);
  }
};
} // namespace klee

#endif
