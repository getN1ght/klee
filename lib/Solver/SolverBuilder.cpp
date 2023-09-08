#include "SolverBuilder.h"

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include <cstdio>
#include <queue>

using namespace klee;

SolverBuilder::SolverBuilder(const std::vector<ref<SolverTheory>> &theories)
    : orderOfTheories(theories) {
  // TODO: raise error
  if (orderOfTheories.empty()) {
    assert(0);
    exit(1);
  }

  for (size_t pos = 0; pos < orderOfTheories.size(); ++pos) {
    const ref<SolverTheory> &theory = orderOfTheories.at(pos);
    auto [_, ok] = positionsOfTheories.emplace(theory->sort(), pos);
    // TODO: print name of theory and replace assert
    assert(ok && "same theory appeared twice in theories sequence");
  }
}

uint64_t SolverBuilder::positionOf(SolverTheory::Sort sort) const {
  if (positionsOfTheories.count(sort)) {
    return positionsOfTheories.at(sort);
  }
  return orderOfTheories.size();
}

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
  ExprHandleList kidsHandles;
  kidsHandles.reserve(expr->getNumKids());

  SolverTheory::Sort leastCommonSort = SolverTheory::Sort::UNKNOWN;
  uint64_t positionOfLeastCommonSort = orderOfTheories.size();

  /* Figure out the least common sort for kid handles. */
  for (const auto &expr : expr->kids()) {
    ref<ExprHandle> kidHandle = build(expr);
    SolverTheory::Sort builtSort = kidHandle->sort();

    // FIXME: optimize to one preprocessing.
    uint64_t positionOfCurrentSort = positionOf(builtSort);

    // TODO: install LLVM error handler. Do not use asserts, they are awful.
    assert(positionOfCurrentSort != orderOfTheories.size() &&
           "Can not find sort of built expression in specified theories");
    positionOfLeastCommonSort =
        std::min(positionOfCurrentSort, positionOfLeastCommonSort);

    kidsHandles.push_back(kidHandle);
  }

  SolverTheory::Sort leastCommonSort =
      orderOfTheories.at(positionOfLeastCommonSort)->theorySort;
  for (ref<ExprHandle> &kid : kidsHandles) {
    kid = castToTheory(kid, leastCommonSort);
  }

  return theory->translate(expr, kidsHandles);
}

ref<ExprHandle> SolverBuilder::build(const ref<Expr> &expr) {
  for (const auto &theory : orderOfTheories) {
    if (ref<ExprHandle> exprHandle = buildWithTheory(theory, expr)) {
      return exprHandle;
    }
  }
  return nullptr;
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
    const ref<ExprHandle> topCastedHandle = castedHandlesQueue.front();
    castedHandlesQueue.pop();

    if (topCastedHandle->sort() == sort) {
      return topCastedHandle;
    }

    // TODO: Do not dereference 2 times.
    const ref<SolverTheory> &theoryOfTopCastedHandle = topCastedHandle->parent;

    for (const auto &[kind, _] : theoryOfTopCastedHandle->castMapping) {
      if (ref<ExprHandle> nextCastedHandle = topCastedHandle->castTo(kind)) {
        castedHandlesQueue.push(nextCastedHandle);
      }
    }
  }

  return nullptr;
}
