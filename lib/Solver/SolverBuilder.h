#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "klee/ADT/Ref.h"
#include "SolverTheory.h"

#include <memory>
#include <vector>

namespace klee {

class Expr;

/*
 * Base class for handles for all builders.
 * Maintains a inner representation for solver
 * and type of expression inside.
 */
class ExprHandle {
private:
  ExprHandleList sideConstraints;

public:
  const ref<SolverTheory> parent;

public:
  enum SolverKind { Z3, STP, METASMT };

public:
  class ReferenceCounter _refCount;

  SolverTheory::Sort sort() const { return parent->theorySort; }

  ref<ExprHandle> castTo(SolverTheory::Sort targetSort) {
    return parent->castTo(targetSort, this);
  }

  void pushSideConstraint(const ref<ExprHandle> &constraint) {
    sideConstraints.push_back(constraint);
  }

  ExprHandle(const ref<SolverTheory> &parent) : parent(parent) {}
  virtual ~ExprHandle() = default;
};

class SolverBuilder {
  friend class SolverBuilderFactory;

private:
  // exprCache cache;
  const std::vector<ref<SolverTheory>> orderOfTheories;

  // Using for optimization
  std::unordered_map<SolverTheory::Sort, uint64_t> positionsOfTheories;

  uint64_t positionOf(SolverTheory::Sort) const;

  SolverBuilder(const std::vector<ref<SolverTheory>> &);
  ref<ExprHandle> buildWithTheory(const ref<SolverTheory> &theory,
                                  const ref<Expr> &expr);

  ref<ExprHandle> castToTheory(const ref<ExprHandle> &arg,
                               SolverTheory::Sort sort);

public:
  ref<ExprHandle> build(const ref<Expr> &expr);
};
} // namespace klee

#endif
