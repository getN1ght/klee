#include "SolverBuilder.h"

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include <queue>

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

  SolverTheory::Sort leastCommonSort = SolverTheory::Sort::UNKNOWN;
  auto positionOfLeastCommonSort = orderOfTheories.end();

  /* Figure out the least common sort for kid handles. */
  for (const auto &expr : expr->kids()) {
    ref<ExprHandle> kidHandle = build(expr);
    SolverTheory::Sort builtSort = kidHandle->sort;

    auto positionOfCurrentSort =
        std::find_if(orderOfTheories.begin(), orderOfTheories.end(),
                     [builtSort](const ref<SolverTheory> &theory) {
                       return theory->theorySort == builtSort;
                     });
    // TODO: install LLVM error handler. Do not use asserts, they are awful.
    assert(positionOfCurrentSort != orderOfTheories.end() &&
           "Can not find sort of built expression in specified theories");
    positionOfLeastCommonSort =
        std::min(positionOfCurrentSort, positionOfLeastCommonSort);

    kidsHandles.push_back(kidHandle);
  }

  SolverTheory::Sort leastCommonSort = (*positionOfLeastCommonSort)->theorySort;
  for (ref<ExprHandle> &kid : kidsHandles) {
    kid = castToTheory(kid, leastCommonSort);
  }

  return theory->translate(expr, kidsHandles);
}

ref<ExprHandle> SolverBuilder::build(const ref<Expr> &expr) {
  for (const auto &theory : orderOfTheories) {
    ref<ExprHandle> exprHandle = buildWithTheory(theory, expr);

  }
}

/*
 * Casts an expression to specified theory.
 * 
 * Inside implements BFS on theories in order to
 * reach target theory in least possible steps.
 * We may consider transitions between theories as
 * weighted and use graph algorithms in order to
 * find the less expensive path to specified theory.
 */
ref<ExprHandle> SolverBuilder::castToTheory(const ref<ExprHandle> &arg,
                                            SolverTheory::Sort sort) {
  std::queue<ref<ExprHandle>> castedHandlesQueue;
  castedHandlesQueue.push(arg);

  while (!castedHandlesQueue.empty()) {
    ref<ExprHandle> topCastedHandle = castedHandlesQueue.front();
    castedHandlesQueue.pop();
  }

  return nullptr;
}
