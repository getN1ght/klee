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
                                bool &isValid);
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

bool SolverBlueprint::relaxSymcreteConstraints(const Query &query,
                                               Assignment &assignment,
                                               bool &isValid) {
  // Get initial symcrete solution. We will try to relax
  // them in order to achieve `mayBeTrue` solution.
  assignment = cm->get(query.constraints);

  ref<Expr> sizesSumToMinimize = ConstantExpr::create(0, 64);
  std::vector<const Array *> brokenSymcreteArrays;

  ValidityCore validityCore;
  
  bool wereConcretizationsRemoved = true;
  while (wereConcretizationsRemoved) {
    wereConcretizationsRemoved = false;
    if (!solver->impl->computeValidityCore(
            constructConcretizedQuery(query, assignment), validityCore,
            isValid)) {
      return false;
    }
    // No unsat cores were found for the query, so we can try
    // to find new solution.
    if (!isValid) {
      break;
    }

    std::vector<const Array *> currentlyBrokenSymcreteArrays =
        Query(ConstraintSet(validityCore.constraints), validityCore.expr)
            .gatherSymcreteArrays();

    for (unsigned idx = 0, initialSize = currentlyBrokenSymcreteArrays.size();
         idx < initialSize; ++idx) {
      if (ref<SymbolicAllocationSource> allocSource =
              dyn_cast_or_null<SymbolicAllocationSource>(
                  currentlyBrokenSymcreteArrays[idx]->source)) {
        currentlyBrokenSymcreteArrays.push_back(allocSource->linkedArray);
      }
    }

    for (const Array *brokenArray : currentlyBrokenSymcreteArrays) {
      if (!assignment.bindings.count(brokenArray)) {
        continue;
      }

      // Erase bindings from received concretization 
      if (brokenArray->source->isSymcrete()) {
        assignment.bindings.erase(brokenArray);
        brokenSymcreteArrays.push_back(brokenArray);
        wereConcretizationsRemoved = true;
      }
      
      // Add symbolic size to the sum that should be minimized.
      if (brokenArray->source->getKind() ==
          ArraySource::Kind::SymbolicSize) {
        sizesSumToMinimize =
            AddExpr::create(sizesSumToMinimize,
                            Expr::createTempRead(brokenArray, /*FIXME:*/ 64));
      }
    }
  }

  if (isValid) {
    return true;
  }

  ConstraintSet queryConstraints =
      constructConcretizedQuery(query, assignment).constraints;
  queryConstraints.push_back(query.negateExpr().expr);

  ref<ConstantExpr> minimalValueOfSum;
  if (!solver->impl->computeMinimalUnsignedValue(
          Query(queryConstraints, sizesSumToMinimize), minimalValueOfSum)) {
    return false;
  }

  std::vector<std::vector<uint8_t>> brokenSymcretesValues;
  bool hasSolution = false;
  if (!solver->impl->computeInitialValues(
          Query(queryConstraints,
                EqExpr::create(sizesSumToMinimize, minimalValueOfSum))
              .negateExpr(),
          brokenSymcreteArrays, brokenSymcretesValues, hasSolution)) {
    return false;
  }
  assert(hasSolution && "Symcrete values should have concretization after "
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
        char *charAddressIterator = reinterpret_cast<char *>(&address);
        assignment.bindings[dependentAddressArray] = std::vector<uint8_t>(
            charAddressIterator, charAddressIterator + sizeof(address));
      }
    }
  }

  if (!solver->impl->computeValidityCore(
          constructConcretizedQuery(query, assignment), validityCore,
          isValid)) {
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
  if (false) { // Проверка на адекватность
    assert(0 && "Assignment does not contain array with extCall source");
  }
  auto concretizedQuery = constructConcretizedQuery(query, assign);
  ref<SolverResponse> trueResponse, falseResponse;

  if (!solver->impl->computeValidity(concretizedQuery, trueResponse,
                                     falseResponse)) {
    return false;
  }

  std::vector<std::vector<uint8_t>> trueResponseValues, falseResponseValues;

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
    bool isValid = false;
    if (!relaxSymcreteConstraints(query, trueResponseAssignment,
                                  isValid)) {
      return false;
    }
    if (!isValid) {
      cm->add(query.negateExpr(), trueResponseAssignment);
      falseInvalid = false;
    }
  } else {
    cm->add(query.negateExpr(), trueResponseAssignment);
    bool isValid = false;
    if (!relaxSymcreteConstraints(query.negateExpr(), falseResponseAssignment,
                                  isValid)) {
      return false;
    }
    if (!isValid) {
      cm->add(query, falseResponseAssignment);
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
    if (!relaxSymcreteConstraints(query, assign, isValid)) {
      return false;
    }
  }

  if (!isValid) {
    cm->add(query.negateExpr(), assign);
  }

  return true;
}

bool SolverBlueprint::computeValidityCore(const Query &query,
                                     ValidityCore &validityCore,
                                     bool &isValid) {
  Query concretizedQuery =
      constructConcretizedQuery(query, cm->get(query.constraints));
  return solver->impl->computeValidityCore(concretizedQuery, validityCore,
                                           isValid);
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

Solver *createSolverBlueprint(Solver *s, ConcretizationManager *cm, AddressGenerator *ag) {
  return new Solver(new SolverBlueprint(s, cm, ag));
}
}
