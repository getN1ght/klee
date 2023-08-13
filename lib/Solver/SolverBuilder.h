#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include <memory>
#include <vector>

namespace klee {

class SolverBuilderFactory;
class SolverTheory;

template <typename> class ref;
class Expr;


/*
 * Base class for handles for all builders.
 * Maintains a inner representation for solver
 * and type of expression inside.
 */
class ExprHandle {
private:
  SolverTheory::Kind theory;

public:
  SolverTheory::Kind type() const {
    return theory;
  }
};


class SolverBuilder {
  friend class SolverBuilderFactory;

private:
  std::vector<std::shared_ptr<SolverTheory>> orderOfTheories;

  SolverBuilder(const std::vector<std::shared_ptr<SolverTheory>> &);

public:
  ExprHandle build(const ref<Expr> &expr);
};
} // namespace klee

#endif
