#include "SolverBuilder.h"

#include "SolverAdapter.h"
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
    [[maybe_unused]] bool ok =
        positionsOfTheories.emplace(theory->theorySort, pos).second;
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

ref<TheoryHandle>
SolverBuilder::buildWithTheory(const ref<SolverTheory> &theory,
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
  TheoryHandleList kidsHandles;
  kidsHandles.reserve(expr->getNumKids());

  llvm::errs() << "IN " << expr << "\n";
  if (expr->getNumKids() == 0) {
    llvm::errs() << "OUT " << expr << "\n";
    return theory->translate(expr, kidsHandles);
  }

  uint64_t positionOfLeastCommonSort = orderOfTheories.size();

  /* Figure out the least common sort for kid handles. */
  

  for (const auto &child : expr->kids()) {
    ref<TheoryHandle> kidHandle = build(child);
    if (ref<BrokenTheoryHandle> brokenKidHandle =
            dyn_cast<BrokenTheoryHandle>(kidHandle)) {
      return kidHandle;
    }

    uint64_t positionOfCurrentSort = positionOf(kidHandle->parent->getSort());

    // TODO: install LLVM error handler. Do not use asserts, they are awful.
    assert(positionOfCurrentSort != orderOfTheories.size() &&
           "Can not find sort of built expression in specified theories");
    positionOfLeastCommonSort =
        std::min(positionOfCurrentSort, positionOfLeastCommonSort);

    kidsHandles.push_back(kidHandle);
  }

  SolverTheory::Sort leastCommonSort =
      orderOfTheories.at(positionOfLeastCommonSort)->theorySort;
  for (ref<TheoryHandle> &kid : kidsHandles) {
    kid = castToTheory(kid, leastCommonSort);
    if (kid.isNull()) {
      llvm::errs() << "WARNING: casted to nullptr\n";
    }

  }

  llvm::errs() << "OUT " << expr << "\n";
  return theory->translate(expr, kidsHandles);
}

ref<TheoryHandle> SolverBuilder::build(const ref<Expr> &expr) {
  if (cache.count(expr)) {
    return cache.at(expr);
  }

  for (const auto &theory : orderOfTheories) {
    ref<TheoryHandle> exprHandle = buildWithTheory(theory, expr);
    if (!isa<BrokenTheoryHandle>(exprHandle)) {
      // TODO: if exprHandle is incomplete, then we should subscribe
      // on it in order to wait until iw will be constructed and then
      // add it to cache.
      cache.emplace(expr, exprHandle);
      return exprHandle;
    }
  }
  cache.emplace(expr, new BrokenTheoryHandle(expr));
  return cache.at(expr);
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
ref<TheoryHandle> SolverBuilder::castToTheory(const ref<TheoryHandle> &arg,
                                              SolverTheory::Sort sort) {
  std::queue<ref<TheoryHandle>> castedHandlesQueue;
  castedHandlesQueue.push(arg); 

  std::unordered_set<SolverTheory::Sort> visited;
  visited.insert(arg->parent->getSort());

  /* FIXME: We should mark visited theories */
  while (!castedHandlesQueue.empty()) {
    const ref<TheoryHandle> topCastedHandle = castedHandlesQueue.front();
    castedHandlesQueue.pop();

    if (topCastedHandle->parent->getSort() == sort) {
      return topCastedHandle;
    }

    // TODO: Do not dereference twice.
    const ref<SolverTheory> &theoryOfTopCastedHandle = topCastedHandle->parent;

    for (const auto &[toSort, _] : theoryOfTopCastedHandle->castMapping) {
      if (visited.count(toSort)) {
        continue;
      }

      if (ref<TheoryHandle> nextCastedHandle =
              topCastedHandle->parent->castTo(toSort, topCastedHandle)) {
        castedHandlesQueue.push(nextCastedHandle);
        visited.insert(toSort);
      }
    }
  }

  llvm::errs() << "Can not cast to " << sort << "\n";

  std::abort();
  return nullptr;
}
