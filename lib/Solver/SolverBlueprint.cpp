#include "klee/ADT/SparseStorage.h"
#include "klee/Expr/Assignment.h"
#include "klee/Expr/Constraints.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprUtil.h"
#include "klee/Expr/IndependentSet.h"
#include "klee/Expr/SymbolicSource.h"

#include "klee/Solver/AddressGenerator.h"
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
  AddressGenerator *ag;

public:
  SolverBlueprint(Solver *_solver, ConcretizationManager *_cm,
                  AddressGenerator *_ag)
      : solver(_solver), cm(_cm), ag(_ag) {}

  ~SolverBlueprint() {
    delete solver;
  }

  bool computeTruth(const Query &, bool &isValid);
  bool computeValidity(const Query &, Solver::Validity &result);
  bool computeValidityCore(const Query &query, ValidityCore &validityCore,
                           bool &isValid);
  bool check(const Query &query, ref<SolverResponse> &result);
  bool computeValue(const Query &, ref<Expr> &result);
  bool computeInitialValues(const Query &query,
                            const std::vector<const Array *> &objects,
                            std::vector<SparseStorage<unsigned char>> &values,
                            bool &hasSolution);
  SolverRunStatus getOperationStatusCode();
  char *getConstraintLog(const Query &);
  void setCoreSolverTimeout(time::Span timeout);

private:
  bool assertConcretization(const Query &query, const Assignment &assign) const;
  bool relaxSymcreteConstraints(const Query &query, Assignment &assign,
                                ref<SolverResponse> &result);
  Query constructConcretizedQuery(const Query &, const Assignment &);
};

Query SolverBlueprint::constructConcretizedQuery(const Query &query,
                                               const Assignment &assign) {
  ConstraintSet constraints = assign.createConstraintsFromAssignment();
  for (auto e : query.constraints) {
    constraints.push_back(e);
  }
  return Query(constraints, query.expr);
}

bool SolverBlueprint::assertConcretization(const Query &query,
                                           const Assignment &assign) const {
  for (const Array *symcreteArray : query.gatherSymcreteArrays()) {
    if (!assign.bindings.count(symcreteArray)) {
      return false;
    }
  }
  return true;
}

bool SolverBlueprint::relaxSymcreteConstraints(const Query &query,
                                               Assignment &assignment,
                                               ref<SolverResponse> &result) {
  // Get initial symcrete solution. We will try to relax
  // them in order to achieve `mayBeTrue` solution.
  assignment = cm->get(query.constraints);

  std::vector<const Array *> brokenSymcreteArrays;
  std::vector<const Array *> brokenSizesArrays;
  
  bool wereConcretizationsRemoved = true;
  while (wereConcretizationsRemoved) {
    wereConcretizationsRemoved = false;
    if (!solver->impl->check(constructConcretizedQuery(query, assignment),
                             result)) {
      return false;
    }
    // No unsat cores were found for the query, so we can try
    // to find new solution.
    if (isa<InvalidResponse>(result)) {
      break;
    }

    ValidityCore validityCore;
    assert(result->getValidityCore(validityCore));
    std::vector<const Array *> currentlyBrokenSymcreteArrays =
        Query(ConstraintSet(validityCore.constraints), validityCore.expr)
            .gatherSymcreteArrays();


    for (const Array *brokenArray : currentlyBrokenSymcreteArrays) {
      if (!assignment.bindings.count(brokenArray)) {
        continue;
      }

      if (ref<SymbolicSizeSource> sizeSource =
              dyn_cast<SymbolicSizeSource>(brokenArray->source)) {
        // Remove size concretization
        assignment.bindings.erase(brokenArray);
        brokenSymcreteArrays.push_back(brokenArray);

        // Remove address concretization
        brokenSymcreteArrays.push_back(sizeSource->linkedArray);
        assignment.bindings.erase(sizeSource->linkedArray);

        wereConcretizationsRemoved = true;
        brokenSizesArrays.push_back(brokenArray);
        // Add symbolic size to the sum that should be minimized.
      }
    }
  }

  if (isa<ValidResponse>(result)) {
    return true;
  }

  Query concretizedNegatedQuery =
      constructConcretizedQuery(query.negateExpr(), assignment);

  ConstraintSet queryConstraints = concretizedNegatedQuery.constraints;
  ConstraintManager queryConstraintsManager(queryConstraints);
  queryConstraintsManager.addConstraint(concretizedNegatedQuery.expr);

  assert(!brokenSizesArrays.empty());
  uint64_t overflowBits = sizeof(unsigned long long) * CHAR_BIT - 1 -
                          __builtin_clzll(brokenSizesArrays.size());
  Expr::Width exprWidth =
      brokenSizesArrays.front()->size->getWidth();
  ref<Expr> sizesSumToMinimize =
      ConstantExpr::create(0, exprWidth + overflowBits);
  for (const Array *array : brokenSizesArrays) {
    sizesSumToMinimize =
        AddExpr::create(sizesSumToMinimize,
                        ZExtExpr::create(Expr::createTempRead(array, exprWidth),
                                         exprWidth + overflowBits));
  }
  sizesSumToMinimize =
      ConstraintManager::simplifyExpr(query.constraints, sizesSumToMinimize);

  ref<ConstantExpr> minimalValueOfSum;
  if (!solver->impl->computeMinimalUnsignedValue(
          Query(queryConstraints, sizesSumToMinimize), minimalValueOfSum)) {
    return false;
  }

  std::vector<SparseStorage<unsigned char>> brokenSymcretesValues;
  bool hasSolution = false;
  if (!solver->impl->computeInitialValues(
          Query(queryConstraints,
                EqExpr::create(sizesSumToMinimize, minimalValueOfSum))
              .negateExpr(),
          brokenSymcreteArrays, brokenSymcretesValues, hasSolution)) {
    return false;
  }
  assert(hasSolution && "Symcretes values should have concretization after "
                        "computeInitialValues() query.");

  for (unsigned idx = 0; idx < brokenSymcreteArrays.size(); ++idx) {
    if (brokenSymcreteArrays[idx]->source->getKind() ==
        ArraySource::Kind::SymbolicSize) {      
      assignment.bindings[brokenSymcreteArrays[idx]] =
          brokenSymcretesValues[idx];
      // Receive address array linked with this size array to request address
      // concretization.
      if (ref<SymbolicAllocationSource> allocSource =
              dyn_cast_or_null<SymbolicAllocationSource>(
                  brokenSymcreteArrays[idx]->source)) {
        const Array *dependentAddressArray = allocSource->linkedArray;

        uint64_t newSize =
            cast<ConstantExpr>(assignment.evaluate(Expr::createTempRead(
                                   brokenSymcreteArrays[idx], 64)))
                ->getZExtValue();

        void *address = ag->allocate(dependentAddressArray, newSize);
        assert(address);
        char *charAddressIterator = reinterpret_cast<char *>(&address);
        assignment.bindings[dependentAddressArray] =
            SparseStorage<unsigned char>(
                std::vector<unsigned char>(
                    charAddressIterator, charAddressIterator + sizeof(address)),
                '\0');
      }
    }
  }

  if (!solver->impl->check(constructConcretizedQuery(query, assignment),
                           result)) {
    return false;
  }

  return true;
}

bool SolverBlueprint::computeValidity(const Query &query,
                                    Solver::Validity &result) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeValidity(query, result);
  }
  auto assign = cm->get(query.constraints);
  assert(assertConcretization(query, assign) &&
         "Assignment does not contain concretization for all symcrete arrays!");

  auto concretizedQuery = constructConcretizedQuery(query, assign);
  ref<SolverResponse> trueResponse, falseResponse;

  if (!solver->impl->computeValidity(concretizedQuery, trueResponse,
                                     falseResponse)) {
    return false;
  }

  std::vector<SparseStorage<unsigned char>> trueResponseValues, falseResponseValues;

  std::vector<const Array *> objects = query.gatherSymcreteArrays();

  bool trueInvalid =
      trueResponse->getInitialValuesFor(objects, trueResponseValues);
  bool falseInvalid =
      falseResponse->getInitialValuesFor(objects, falseResponseValues);
  assert(trueInvalid || falseInvalid);

  Assignment trueResponseAssignment(true), falseResponseAssignment(true);
  if (trueInvalid) {
    trueResponseAssignment = Assignment(objects, trueResponseValues, true);
  }
  if (falseInvalid) {
    falseResponseAssignment = Assignment(objects, falseResponseValues, true);
  }

  // *No more than one* of trueResponse and falseResponse is possible,
  // i.e. `mustBeTrue` with values from `assign`.
  // Take one which is `mustBeTrue` with symcretes from `assign`
  // and try to relax them to `mayBeFalse`. This solution should be
  // appropriate for the remain branch.

  if (!trueInvalid) {
    cm->add(query, falseResponseAssignment);
    ref<SolverResponse> solverResponse;
    if (!relaxSymcreteConstraints(query, trueResponseAssignment,
                                  solverResponse)) {
      return false;
    }
    if (!isa<ValidResponse>(solverResponse)) {
      cm->add(query.negateExpr(), trueResponseAssignment);
      falseInvalid = false;
    }
  } else if (!falseInvalid) {
    cm->add(query.negateExpr(), trueResponseAssignment);
    ref<SolverResponse> solverResponse;
    if (!relaxSymcreteConstraints(query.negateExpr(), falseResponseAssignment,
                                  solverResponse)) {
      return false;
    }
    if (!isa<ValidResponse>(solverResponse)) {
      cm->add(query, falseResponseAssignment);
      trueInvalid = false;
    }
  }

  result = (Solver::Validity)((!trueInvalid) - (!falseInvalid));
  return true;
}

bool SolverBlueprint::check(const Query &query, ref<SolverResponse> &result) {
  if (!query.containsSymcretes()) {
    return solver->impl->check(query, result);
  }
  auto assign = cm->get(query.constraints);
  assert(assertConcretization(query, assign) &&
         "Assignment does not contain concretization for all symcrete arrays!");

  auto concretizedQuery = constructConcretizedQuery(query, assign);
  if (!solver->impl->check(concretizedQuery, result)) {
    return false;
  }

  if (isa<ValidResponse>(result)) {
    if (!relaxSymcreteConstraints(query, assign, result)) {
      return false;
    }
  }

  if (isa<InvalidResponse>(result)) {
    cm->add(query.negateExpr(), assign);
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
  assert(assertConcretization(query, assign) &&
         "Assignment does not contain concretization for all symcrete arrays!");

  if (ref<ConstantExpr> CE = dyn_cast<ConstantExpr>(
          cm->simplifyExprWithSymcretes(query.constraints, query.expr))) {
    isValid = CE->isTrue(); 
  } else {
    auto concretizedQuery = constructConcretizedQuery(query, assign);
    if (!solver->impl->computeTruth(concretizedQuery, isValid)) {
      return false;
    }
  }

  // If constraints always evaluate to `mustBeTrue`, then relax
  // symcretes until remove all of them or query starts to evaluate
  // to `mayBeFalse`.

  if (isValid) {
    ref<SolverResponse> solverResponse;
    if (!relaxSymcreteConstraints(query, assign, solverResponse)) {
      return false;
    }
    isValid = isa<ValidResponse>(solverResponse);
  }

  if (!isValid) {
    cm->add(query.negateExpr(), assign);
  }

  return true;
}

bool SolverBlueprint::computeValidityCore(const Query &query,
                                          ValidityCore &validityCore,
                                          bool &isValid) {
  Assignment assign = cm->get(query.constraints);
  assert(assertConcretization(query, assign) &&
         "Assignment does not contain concretization for all symcrete arrays!");

  Query concretizedQuery = constructConcretizedQuery(query, assign);
  
  if (ref<ConstantExpr> CE = dyn_cast<ConstantExpr>(
          cm->simplifyExprWithSymcretes(query.constraints, query.expr))) {
    isValid = CE->isTrue();
  } else {
    if (!solver->impl->computeValidityCore(concretizedQuery, validityCore,
                                          isValid)) {
      return false;
    }
  }
  

  if (isValid) {
    ref<SolverResponse> solverResponse;
    if (!relaxSymcreteConstraints(query, assign, solverResponse)) {
      return false;
    }
    /* Here we already have validity core from query above. */
    if (solverResponse->getValidityCore(validityCore)) {
      isValid = false;
    }
  }

  if (!isValid) {
    validityCore = ValidityCore();
    cm->add(query.negateExpr(), assign);
  }

  return true;
}

bool SolverBlueprint::computeValue(const Query &query, ref<Expr> &result) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeValue(query, result);
  }

  Assignment assign = cm->get(query.constraints);
  assert(assertConcretization(query, assign) &&
         "Assignment does not contain concretization for all symcrete arrays!");

  if (ref<ConstantExpr> expr = dyn_cast<ConstantExpr>(
          cm->simplifyExprWithSymcretes(query.constraints, query.expr))) {
    result = expr;
    return true;
  }
  auto concretizedQuery = constructConcretizedQuery(query, assign);
  return solver->impl->computeValue(concretizedQuery, result);
}

bool SolverBlueprint::computeInitialValues(
    const Query &query, const std::vector<const Array *> &objects,
    std::vector<SparseStorage<unsigned char>> &values, bool &hasSolution) {
  if (!query.containsSymcretes()) {
    return solver->impl->computeInitialValues(query, objects, values,
                                              hasSolution);
  }

  Assignment assign = cm->get(query.constraints);
  assert(assertConcretization(query, assign) &&
         "Assignment does not contain concretization for all symcrete arrays!");

  auto concretizedQuery = constructConcretizedQuery(query, assign);
  if (!solver->impl->computeInitialValues(concretizedQuery, objects, values,
                                         hasSolution)) {
    return false;
  }

  if (!hasSolution) {
    ref<SolverResponse> solverResponse;
    if (!relaxSymcreteConstraints(query, assign, solverResponse)) {
      return false;
    }
    /* Because relaxSymcreteConstraints response is `isValid`,
    and `isValid` == false iff solution for negation exists. */
    hasSolution = isa<InvalidResponse>(solverResponse);
    if (hasSolution) {
      cm->add(query.negateExpr(), assign);
      values = std::vector<SparseStorage<unsigned char>>();
      return solver->impl->computeInitialValues(
          constructConcretizedQuery(query, assign), objects, values,
          hasSolution);
    }
  }

  return true;
}

// Redo later
SolverImpl::SolverRunStatus SolverBlueprint::getOperationStatusCode() {
  return solver->impl->getOperationStatusCode();
}


void SolverBlueprint::setCoreSolverTimeout(time::Span timeout) {
  solver->setCoreSolverTimeout(timeout);
}

Solver *createSolverBlueprint(Solver *s, ConcretizationManager *cm, AddressGenerator *ag) {
  return new Solver(new SolverBlueprint(s, cm, ag));
}
}
