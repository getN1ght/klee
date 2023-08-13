#include "SolverBuilder.h"

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

using namespace klee;

SolverBuilder::SolverBuilder(
    const std::vector<std::shared_ptr<SolverTheory>> &theories)
    : orderOfTheories(theories) {}

ExprHandle SolverBuilder::build(const ref<Expr> &expr) {
  std::vector<ExprHandle> kidsHandles;
  kidsHandles.reserve(expr->getNumKids());

  SolverTheory::Kind leastCommonType;
  for (const auto &expr : expr->kids()) {
    std::optional<ExprHandle> kidHandle = build(expr);
    kidsHandles.push_back(kidHandle.value());
  }

  for (const auto &theory : orderOfTheories) {
    std::optional<ExprHandle> handle = theory->translate(expr);
    if (handle.has_value()) {
      return handle.value();
    }

    /* If handle is empty switch to another theory. */
  }
}