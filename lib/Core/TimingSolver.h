//===-- TimingSolver.h ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_TIMINGSOLVER_H
#define KLEE_TIMINGSOLVER_H

#include "klee/Expr/Constraints.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprHashMap.h"
#include "klee/Expr/Assignment.h"
#include "klee/Solver/Solver.h"
#include "klee/System/Time.h"

#include <unordered_map>
#include <memory>
#include <vector>

namespace klee {
class ConstraintSet;
class Solver;
class ExecutionState;

/// TimingSolver - A simple class which wraps a solver and handles
/// tracking the statistics that we care about.
class TimingSolver {
public:
  std::unique_ptr<Solver> solver;
  bool simplifyExprs;

public:
  /// TimingSolver - Construct a new timing solver.
  ///
  /// \param _simplifyExprs - Whether expressions should be
  /// simplified (via the constraint manager interface) prior to
  /// querying.
  TimingSolver(Solver *_solver, bool _simplifyExprs = true)
      : solver(_solver), simplifyExprs(_simplifyExprs) {}

  void setTimeout(time::Span t) { solver->setCoreSolverTimeout(t); }

  char *getConstraintLog(const Query &query) {
    return solver->getConstraintLog(query);
  }

  bool evaluate(const ConstraintSet &, ref<Expr>, Solver::Validity &result,
                SolverQueryMetaData &metaData, bool produceValidityCore = false);

  bool evaluate(const ConstraintSet &, ref<Expr>,
                ref<SolverResponse> &queryResult,
                ref<SolverResponse> &negateQueryResult,
                SolverQueryMetaData &metaData);

  bool mustBeTrue(ExecutionState &state, const ConstraintSet &, ref<Expr>,
                  bool &result, SolverQueryMetaData &metaData,
                  Assignment &symcretesCex, bool minimizeModel = false);

  bool mustBeFalse(ExecutionState &state, const ConstraintSet &, ref<Expr>,
                   bool &result, SolverQueryMetaData &metaData,
                   Assignment &symcretesCex, bool minimizeModel = false);

  bool mayBeTrue(ExecutionState &state, const ConstraintSet &, ref<Expr>,
                 bool &result, SolverQueryMetaData &metaData,
                 Assignment &symcretesEx, bool minimizeModel = false);

  bool mayBeFalse(ExecutionState &state, const ConstraintSet &, ref<Expr>,
                  bool &result, SolverQueryMetaData &metaData,
                  Assignment &symcretesEx, bool minimizeModel = false);

  bool getValue(const ConstraintSet &, ref<Expr> expr,
                ref<ConstantExpr> &result, SolverQueryMetaData &metaData);

  bool getInitialValues(const ConstraintSet &,
                        const std::vector<const Array *> &objects,
                        std::vector<std::vector<unsigned char>> &result,
                        SolverQueryMetaData &metaData,
                        bool produceValidityCore = false);

  bool getValidityCore(const ConstraintSet &, ref<Expr>,
                       ValidityCore &validityCore, bool &result,
                       SolverQueryMetaData &metaData);

  bool getValidAssignment(const ConstraintSet &, ref<Expr>,
                          ValidityCore validityCore,
                          Assignment symcretes,
                          const std::unordered_map<const Array *, MemoryObject *> &symsizes,
                          ExprHashMap<std::set<const Array *>> &exprToSymcretes,
                          bool &hasResult,
                          Assignment &result,
                          SolverQueryMetaData &metaData, bool minimizeModel = false) const;

  std::pair<ref<Expr>, ref<Expr>> getRange(const ConstraintSet &,
                                           ref<Expr> query,
                                           SolverQueryMetaData &metaData,
                                           time::Span timeout = time::Span());
};
}

#endif /* KLEE_TIMINGSOLVER_H */
