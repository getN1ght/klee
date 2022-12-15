#include "klee/Expr/Assignment.h"
#include "klee/Expr/Constraints.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprUtil.h"
#include "klee/Expr/IndependentSet.h"
#include "klee/Expr/SymbolicSource.h"
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
  bool relaxSymcreteConstraints(const Query &query, Assignment &assign,
                                bool &canBeRelaxed);
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

bool SolverBlueprint::relaxSymcreteConstraints(const Query &query,
                                               Assignment &assignment,
                                               bool &canBeRelaxed) {
  // Get initial symcrete solution. We will try to relax
  // them in order to achieve `mayBeTrue` solution.
  assignment = cm->get(query.constraints);

  ref<Expr> sizesSumToMinimize = ConstantExpr::create(0, 64);
  std::vector<const Array *> brokenSymcreteArrays;

  ValidityCore validityCore;
  
  while (true) {
    if (!solver->impl->computeValidityCore(
            constructConcretizedQuery(query.withValidityCore(),
                                      assignment),
            validityCore, canBeRelaxed)) {
      return false;
    }

    // No unsat cores were found for the query, so we can try
    // to find new solution.
    if (!canBeRelaxed) {
      canBeRelaxed = true;
      break;
    }

    std::vector<const Array *> currentlyBrokenSymcreteArrays =
        Query(ConstraintSet(validityCore.constraints), validityCore.expr)
            .gatherArrays();

    // If we could relax constraints before, but constraints from
    // unsat core do not contain symcrete arrays, then relaxation is impossible.
    if (currentlyBrokenSymcreteArrays.empty()) {
      canBeRelaxed = false;
      return true;
    }

    for (unsigned idx = 0, initialSize = currentlyBrokenSymcreteArrays.size();
         idx < initialSize; ++idx) {
      for (const Array *dependent :
           currentlyBrokenSymcreteArrays[idx]->getInderectlyDependentArrays()) {
        currentlyBrokenSymcreteArrays.push_back(dependent);
      }
    }

    for (const Array *brokenArray : currentlyBrokenSymcreteArrays) {
      if (!assignment.bindings.count(brokenArray)) {
        continue;
      }

      // Erase bindings from received concretization 
      if (brokenArray->source->isSymcrete()) {
        assignment.bindings.erase(brokenArray);
      }

      // Add symbolic size to the sum that should be minimized.
      
      // TODO: move `Context.h`
      // TODO: link size with address
      if (brokenArray->source->getKind() ==
          SymbolicSource::Kind::SymbolicSize) {
        sizesSumToMinimize =
            AddExpr::create(sizesSumToMinimize,
                            Expr::createTempRead(brokenArray, /*FIXME:*/ 64));
      }
    }

    brokenSymcreteArrays.insert(brokenSymcreteArrays.end(),
                                currentlyBrokenSymcreteArrays.begin(),
                                currentlyBrokenSymcreteArrays.end());
  }

  // TODO: Add constraints to bound sizes
  ConstraintSet queryConstraints =
      constructConcretizedQuery(query, assignment).constraints;
  queryConstraints.push_back(query.negateExpr().expr);

  ref<ConstantExpr> minimalValueOfSum;
  if (!solver->impl->computeMinimalUnsignedValue(Query(queryConstraints, sizesSumToMinimize), minimalValueOfSum)) {
    return false;
  }

  std::vector<std::vector<uint8_t>> brokenSymcretesValues;
  bool hasSolution = false;
  if (!solver->impl->computeInitialValues(
          Query(queryConstraints,
                EqExpr::create(sizesSumToMinimize, minimalValueOfSum)),
          brokenSymcreteArrays, brokenSymcretesValues, hasSolution)) {
    return false;
  }
  assert(hasSolution && "Symcrete values should have concretization after "
                        "computeInitialValues() query.");

  for (unsigned idx = 0; idx < brokenSymcreteArrays.size(); ++idx) {
    if (brokenSymcreteArrays[idx]->source->getKind() ==
        SymbolicSource::Kind::SymbolicSize) {
      assignment.bindings[brokenSymcreteArrays[idx]] =
          brokenSymcretesValues[idx];
    }
  }

  return true;
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

  if (!trueInvalid) {
    cm->add(query, trueResponseAssignment);
    bool canBeRelaxed = false;
    if (!relaxSymcreteConstraints(query, falseResponseAssignment,
                                  canBeRelaxed)) {
      return false;
    }
    if (canBeRelaxed) {
      cm->add(query.negateExpr(), falseResponseAssignment);
      falseInvalid = false;
    }
  } else {
    cm->add(query.negateExpr(), falseResponseAssignment);
    bool canBeRelaxed = false;
    if (!relaxSymcreteConstraints(query.negateExpr(), trueResponseAssignment,
                                  canBeRelaxed)) {
      return false;
    }
    if (canBeRelaxed) {
      cm->add(query, trueResponseAssignment);
      trueInvalid = false;
    }
  }

  result = (Solver::Validity)((!trueInvalid) - (!falseInvalid));
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
    if (!relaxSymcreteConstraints(query, assign, isValid)) {
      return false;
    }
    isValid = !isValid;
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
