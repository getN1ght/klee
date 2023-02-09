#include "klee/Solver/ConcretizationManager.h"
#include "klee/Expr/Assignment.h"
#include "klee/Expr/Constraints.h"
#include "klee/Expr/IndependentSet.h"
#include "klee/Solver/Solver.h"
#include <set>

using namespace klee;

Assignment ConcretizationManager::get(const ConstraintSet &set,
                                      ref<Expr> query) {
  if (/*simplifyExprs*/ true)
    query = ConstraintManager::simplifyExpr(set, query);
  
  CacheEntry ce(set, query);
  concretizations_map::iterator it = concretizations.find(ce);
  if (it != concretizations.end()) {
    return it->second;
  } else if (!set.empty()) {
    assert(0);
  }

  return Assignment(true);
}

bool ConcretizationManager::contains(const ConstraintSet &set,
                                     ref<Expr> query) {
  if (/*simplifyExprs*/ true)
    query = ConstraintManager::simplifyExpr(set, query);
  CacheEntry ce(set, query);
  concretizations_map::iterator it = concretizations.find(ce);
  return it != concretizations.end();
}

void ConcretizationManager::add(const Query &query, const Assignment &assign) {
  ref<Expr> expr = query.expr;
  if (/*simplifyExprs*/ true)
    expr = ConstraintManager::simplifyExpr(query.constraints, expr);
  CacheEntry ce(query.constraints, query.expr);
  concretizations.insert(std::make_pair(ce, assign));
}

ref<Expr>
ConcretizationManager::simplifyExprWithSymcretes(const ConstraintSet &cs,
                                                 ref<Expr> e) {
  const Assignment &assign = cs.getConcretization();
  ConstraintSet newCS = assign.createConstraintsFromAssignment();
  ConstraintManager cm(newCS);
  for (ref<Expr> constraint : cs) {
    cm.addConstraint(assign.evaluate(constraint));
  }
  return ConstraintManager::simplifyExpr(newCS, e);
}
