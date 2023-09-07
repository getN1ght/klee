#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "klee/ADT/Ref.h"

#include <memory>
#include <vector>

namespace klee {

class SolverTheory;

template <typename> class ref;
class Expr;

/*
 * Base class for handles for all builders.
 * Maintains a inner representation for solver
 * and type of expression inside.
 */
class ExprHandle {
public:
  enum SolverKind { Z3, STP, METASMT };

public:
  class ReferenceCounter _refCount;

  const SolverTheory::Sort sort;
  // SolverTheory::Kind type() const {
  //   return theory;
  // }
  virtual ~ExprHandle() = default;
};

class SolverBuilder {
  friend class SolverBuilderFactory;

private:
  // exprCache cache;
  std::vector<ref<SolverTheory>> orderOfTheories;

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
