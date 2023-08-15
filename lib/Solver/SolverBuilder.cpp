#include "SolverBuilder.h"

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

using namespace klee;

SolverBuilder::SolverBuilder(
    const std::vector<std::shared_ptr<SolverTheory>> &theories)
    : orderOfTheories(theories) {}

ref<ExprHandle> SolverBuilder::build(const ref<Expr> &expr) {
  std::vector<ref<ExprHandle>> kidsHandles;
  kidsHandles.reserve(expr->getNumKids());

  for (const auto &expr : expr->kids()) {
    ref<ExprHandle> kidHandle = build(expr);
    kidsHandles.push_back(kidHandle);
  }

  for (const auto &theory : orderOfTheories) {
    ref<ExprHandle> handle = theory->translate(expr);
    // if (handle()) {
    //   return handle.value();
    // }

    /* If handle is empty switch to another theory. */
  }
}