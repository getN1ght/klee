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
  enum Kind {
    Z3,
    STP,
    METASMT
  };
private:
  /* TODO: it is a sort. */
  // SolverTheory::Kind theory;

public:
  class ReferenceCounter _refCount;
  // SolverTheory::Kind type() const {
  //   return theory;
  // }
  virtual ~ExprHandle() = default;
};


class SolverBuilder {
  friend class SolverBuilderFactory;

private:
  // exprCache cache;
  std::vector<std::shared_ptr<SolverTheory>> orderOfTheories;

  SolverBuilder(const std::vector<std::shared_ptr<SolverTheory>> &);

public:
  ref<ExprHandle> build(const ref<Expr> &expr);
};
} // namespace klee

#endif
