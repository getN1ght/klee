#include "SolverBuilder.h"

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

using namespace klee;

SolverBuilder::SolverBuilder(const std::vector<ref<SolverTheory>> &theories)
    : orderOfTheories(theories) {}

ref<ExprHandle> SolverBuilder::buildWithTheory(const ref<SolverTheory> &theory,
                                               const ref<Expr> &expr) {
  /*
   * Translates `klee::Expr` using given theory.
   * Firstly, translates all children of given expression.
   * Then, strategy may differ:
   * - we may choose the lowest common sort from constructed
   *   expressions, and make a casts to it
   * - we may rebuild children in the sort of the lowest
   *   common sort
   */
  ArgumentsList kidsHandles;
  kidsHandles.reserve(expr->getNumKids());

  for (const auto &expr : expr->kids()) {
    ref<ExprHandle> kidHandle = build(expr);
    kidsHandles.push_back(kidHandle);
  }

  return theory->translate(expr, kidsHandles);
}

ref<ExprHandle> SolverBuilder::build(const ref<Expr> &expr) {
  for (const auto &theory : orderOfTheories) {
    ref<ExprHandle> exprHandle = buildWithTheory(theory, expr);
    /* We may do upcasts from the lowest theory */

    if (exprHandle) {
    }
  }
}
