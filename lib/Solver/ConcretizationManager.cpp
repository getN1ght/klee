#include "klee/Solver/ConcretizationManager.h"
#include "klee/Expr/Assignment.h"
#include "klee/Expr/Constraints.h"
#include "klee/Expr/IndependentSet.h"
#include "klee/Solver/Solver.h"
#include <set>

using namespace klee;

Assignment ConcretizationManager::get(const ConstraintSet &set) {
  Assignment assign(true);
  auto independent = getAllIndependentConstraintsSets(
      Query(set, ConstantExpr::alloc(0, Expr::Bool)));
  for (auto i : *independent) {
    if (auto a = concretizations.lookup(ConstraintSet(i.exprs).asSet())) {
      for (auto i : a->bindings) {
        assign.bindings.insert(i);
      }
    }
  }

  return assign;
}

void ConcretizationManager::add(const ConstraintSet &oldCS,
                                const ConstraintSet &newCS,
                                const Assignment &assign) {
  Assignment newAssign(true);
  ref<Expr> newExpr = ConstantExpr::alloc(1, Expr::Bool);
  for (auto i : newCS) {
    newExpr = AndExpr::alloc(newExpr, i);
  }
  std::vector<ref<Expr>> dependent;
  auto dependency = getIndependentConstraints(Query(oldCS, newExpr), dependent);
  auto independent = getAllIndependentConstraintsSets(
      Query(oldCS, ConstantExpr::alloc(0, Expr::Bool)));

  for (auto i : *independent) {
    if (i.intersects(dependency)) {
      if (auto a = concretizations.lookup(ConstraintSet(i.exprs).asSet())) {
        for (auto i : a->bindings) {
          newAssign.bindings.insert(i);
        }
      }
    }
  }

  std::set<ref<Expr>> dependentWithNew;
  for (auto i : dependent) {
    dependentWithNew.insert(i);
  }

  for (auto i : newCS) {
    dependentWithNew.insert(i);
  }

  for (auto i : assign.bindings) {
    newAssign.bindings.insert(i);
  }

  concretizations.insert(dependentWithNew, newAssign);
}

void ConcretizationManager::add(const Query &q, const Assignment &assign) {
  std::vector<ref<Expr>> dependent;
  getIndependentConstraints(q, dependent);
  ConstraintSet newCS(dependent);
  // q.dump();
  newCS.push_back(q.expr);
  add({}, newCS, assign);
}
