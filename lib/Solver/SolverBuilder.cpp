#include "SolverBuilder.h"

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

using namespace klee;

SolverBuilder::SolverBuilder(const std::vector<ref<SolverTheory>> &theories)
    : orderOfTheories(theories) {}

ref<ExprHandle> SolverBuilder::buildWithTheory(const ref<SolverTheory> &theory,
                                               const ref<Expr> &expr) {
  ArgumentsList kidsHandles;
  kidsHandles.reserve(expr->getNumKids());

  for (const auto &expr : expr->kids()) {
    ref<ExprHandle> kidHandle = build(expr);
    kidsHandles.push_back(kidHandle);
  }

  return theory->translate(expr->getKind(), kidsHandles);
}

ref<ExprHandle> SolverBuilder::build(const ref<Expr> &expr) {
  for (const auto &theory : orderOfTheories) {
    ref<ExprHandle> exprHandle = buildWithTheory(theory, expr);
    if (exprHandle) {

    }
  }
}