#include "klee/Expr/Assignment.h"
#include "klee/Expr/Constraints.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprUtil.h"
#include "klee/Expr/IndependentSet.h"
#include "klee/Solver/ConcretizationManager.h"
#include "klee/Solver/Solver.h"
#include "klee/Solver/SolverImpl.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Casting.h"
#include <algorithm>
#include <vector>

namespace klee {

class SolverBlueprint : public SolverImpl {
private:
  Solver *solver;
  ConcretizationManager *cm;

public:
  SolverBlueprint(Solver *_solver, ConcretizationManager *_cm)
      : solver(_solver), cm(_cm) {}

  ~SolverBlueprint() {
    delete solver;
  }

  bool computeTruth(const Query &, bool &isValid); 
  bool computeValidity(const Query &, Solver::Validity &result);
  bool computeValue(const Query &, ref<Expr> &result);
  bool computeInitialValues(const Query &query,
                            const std::vector<const Array *> &objects,
                            std::vector<std::vector<unsigned char>> &values,
                            bool &hasSolution);
  SolverRunStatus getOperationStatusCode();
  char *getConstraintLog(const Query&);
  void setCoreSolverTimeout(time::Span timeout);

private:
  Query constructConcretizedQuery(const Query &, const Assignment &);
};

Query SolverBlueprint::constructConcretizedQuery(const Query &query,
                                               const Assignment &assign) {
  ConstraintSet constraints;
  for (auto e : query.constraints) {
    constraints.push_back(assign.evaluate(e));
  }
  ref<Expr> expr = assign.evaluate(query.expr);
  ConstraintSet equalities = assign.createConstraintsFromAssignment();
  for (auto e : equalities) {
    constraints.push_back(e);
  }
  return Query(constraints, expr);
}

bool SolverBlueprint::computeValidity(const Query &query,
                                    Solver::Validity &result) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeValidity(query, result);
  }
  auto assign = cm->get(query.constraints);
  if (false) { // Проверка на адекватность
    assert(0 && "Assignment does not contain array with extCall source");
  }
  auto concretizedQuery = constructConcretizedQuery(query, assign);
  ref<SolverResponse> trueResponse, falseResponse;

  if (!solver->impl->computeValidity(concretizedQuery, trueResponse,
                                      falseResponse)) {
    return false;
  }


  Assignment trueResponseAssignment(true), falseResponseAssignment(true);
  
  bool trueInvalid = trueResponse->getInitialValues(trueResponseAssignment.bindings);
  bool falseInvalid = falseResponse->getInitialValues(falseResponseAssignment.bindings);

  // *No more than one* of trueResponse and falseResponse is possible,
  // i.e. `mustBeTrue` with values from `assign`.
  // Take one which is `mustBeTrue` with symcretes from `assign`
  // and try to relax them to `mayBeFalse`. This solution should be
  // appropriate for the remain branch.
  
  // TODO: relax model.
  // TODO: add to the required one relaxed constraints

  // cm->add(query, trueResponseAssignment);
  // cm->add(query.negateExpr(), falseResponseAssignment);
  
  // FIXME: temporary solution
  cm->add(query, assign);
  cm->add(query.negateExpr(), assign);
  
  // result = (Solver::Validity) (falseInvalid - trueInvalid);
  if (!solver->impl->computeValidity(concretizedQuery, result)) {
    return false;
  }

  return true;
}

char *SolverBlueprint::getConstraintLog(const Query &query) {
  return solver->impl->getConstraintLog(query);
}


bool SolverBlueprint::computeTruth(const Query &query, bool &isValid) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeTruth(query, isValid);
  }

  auto assign = cm->get(query.constraints);
  if (false) { // Проверка на адекватность
    assert(0 && "Assignment does not contain array with extCall source");
  }

  auto concretizedQuery = constructConcretizedQuery(query, assign);
  ValidityCore validityCore;

  if (!solver->impl->computeValidityCore(concretizedQuery, validityCore,
                                          isValid)) {
    return false;
  }

  // If constraints always evaluate to `mustBeTrue`, then relax
  // symcretes until remove all of them or query starts to evaluate
  // to `mayBeFalse`.

  if (isValid) {
    cm->add(query, assign);
    // TODO: relax model.
    // TODO: somehow save the solution, try to make isValid = false
  }

  if (!isValid) {
    cm->add(query.negateExpr(), assign);
  }

  return true;
}

bool SolverBlueprint::computeValue(const Query &query, ref<Expr> &result) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeValue(query, result);
  }

  Assignment assign = cm->get(query.constraints);

  if (false) { // Проверка на адекватность
    assert(0 && "External call array is not in assignment");
  }

  auto concretizedQuery = constructConcretizedQuery(query, assign);
  if (ref<ConstantExpr> expr =
          dyn_cast<ConstantExpr>(ConstraintManager::simplifyExpr(
              concretizedQuery.constraints, concretizedQuery.expr))) {
    result = expr;
    return true;
  }
  return solver->impl->computeValue(concretizedQuery, result);
}

bool SolverBlueprint::computeInitialValues(
    const Query &query, const std::vector<const Array *> &objects,
    std::vector<std::vector<unsigned char>> &values, bool &hasSolution) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeInitialValues(query, objects, values,
                                              hasSolution);
  }

  Assignment assign = cm->get(query.constraints);
  if (false) { // Проверка на адекватность
    assert(0 && "External call array is not in assignment");
  }

  auto concretizedQuery = constructConcretizedQuery(query, assign);
  return solver->impl->computeInitialValues(concretizedQuery, objects, values,
                                            hasSolution);
}

// Redo later
SolverImpl::SolverRunStatus SolverBlueprint::getOperationStatusCode() {
  return solver->impl->getOperationStatusCode();
}


void SolverBlueprint::setCoreSolverTimeout(time::Span timeout) {
  solver->setCoreSolverTimeout(timeout);
}

Solver *createSolverBlueprint(Solver *s, ConcretizationManager *cm) {
  return new Solver(new SolverBlueprint(s, cm));
}
}
