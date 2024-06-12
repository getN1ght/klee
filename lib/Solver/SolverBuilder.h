#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "TheoryHandle.h"
#include "klee/ADT/Ref.h"

#include "klee/Expr/ExprHashMap.h"

#include <optional>
#include <variant>

namespace klee {

class Expr;

template <typename... Theory> class SolverBuilder {
  template <typename... RawTheory> friend class RawSolverBuilder;

  typedef std::optional<std::variant<ref<TheoryHandle<Theory>>...>>
      TheoryHandleT;

  typedef ExprHashMap<TheoryHandleT> CacheT;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

private:
  /* Cache for already built expressions */
  CacheT cache;
  ref<SolverAdapter> solver;

  SolverBuilder() {
    static_assert(sizeof...(Theory) != 0,
                  "no theories specified for the builder");
  }

  template <typename RT, typename PT>
  ref<TheoryHandle<RT>> buildWithTheory(const ref<SolverTheory<PT>> &theory,
                                        const ref<Expr> &expr) {
    if (expr->getNumKids() == 0) {
      return theory->translate(expr);
    }

    std::vector<TheoryHandleT> children;
    for (std::size_t i = 0; i < expr->getNumKids(); ++i) {
      children.emplace_back(build(expr->getKid(i)));
    }

    switch (children.size()) {
    case 0:
      return theory->translate(expr);
    case 1:
      return theory->translate(expr, children[0]);
    case 2:
      return theory->translate(expr, children[0], children[1]);
    case 3:
      return theory->translate(expr, children[0], children[1], children[2]);
    default:
      // TODO:
      assert(false);
    }
  }

public:
  /*
   * Translates KLEE's inner representation of expression
   * to the expression for the solver, specified in solverAdapter.
   */
  TheoryHandleT build(const ref<Expr> &expr) {
    if (cache.count(expr)) {
      return cache.at(expr);
    }

    /*
     * Iterating over theories in order to find capable
     * of translating given expression.
     */
    bool hasConstructed = false;
    (
        [&](ref<Theory> theory) -> void {
          if (hasConstructed) {
            return;
          }
          auto exprHandle = buildWithTheory(theory, expr);
          /*
           * If handle is broken, then expression can not be built
           * in that theory. Try another one.
           */
          if (!exprHandle.isNull()) {
            cache.emplace(expr, std::optional{exprHandle});
            hasConstructed = true;
          } else {
            cache.emplace(expr, std::nullopt);
          }
        }(new Theory(solver)),
        ...);

    // /*
    //  * All theories can not be use to translate current expression
    //  * in solver's expression. Memoize that we can not build it
    //  * and return broken handle.
    //  */
    return cache.at(expr);
  }
};
} // namespace klee

#endif
