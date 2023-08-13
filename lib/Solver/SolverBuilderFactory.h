#ifndef SOLVERBUILDERFACTORY_H
#define SOLVERBUILDERFACTORY_H

#include <vector>
#include <type_traits>
#include <memory>

#include "SolverBuilder.h"

class SolverTheory;
class SolverBuilder;
class SolverAdapter;

namespace klee {
class SolverBuilderFactory {
private:
  std::shared_ptr<SolverAdapter> adapter;
  std::vector<std::shared_ptr<SolverTheory>> orderOfTheories;

  SolverBuilderFactory() = default;

public:
  /*
   * Constructs the factory for specified solver.
   * Takes class of adapter. 
   */
  template <typename ST> static SolverBuilderFactory forSolver() {
    static_assert(std::is_base_of_v<SolverAdapter, ST> == true);
    adapter = new ST();
  }

  /* 
   * Queries a theory into the theories sequence. 
   * During builder construction these theories will be applied
   * in order of calls to this method.
   */
  template <typename TT> SolverBuilderFactory thenApply() {
    static_assert(std::is_base_of_v<SolverTheory, TT> == true);
    typename decltype(*adapter)::TT Theory;
    orderOfTheories.emplace_back(new Theory());
  }

  SolverBuilder build() const {
    return SolverBuilder(orderOfTheories);
  }
};

} // namespace klee

#endif // SOLVERBUILDERFACTORY_H
