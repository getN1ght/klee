#ifndef SOLVERBUILDERFACTORY_H
#define SOLVERBUILDERFACTORY_H

#include <memory>
#include <type_traits>
#include <vector>

#include "SolverAdapter.h"
#include "SolverBuilder.h"
#include "SolverTheory.h"



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
    static_assert(std::is_base_of_v<SolverAdapter, ST>,
                  "Solver adapter required to instantiate a factory");
    SolverBuilderFactory solverBuilderFactory;
    solverBuilderFactory.adapter.reset((SolverAdapter *)new ST());
    return solverBuilderFactory;
  }

  /*
   * Queries a theory into the theories sequence.
   * During builder construction these theories will be applied
   * in order of calls to this method.
   */
  template <typename TT> SolverBuilderFactory thenApply() {
    static_assert(std::is_base_of_v<SolverTheory, merger<std::remove_reference<decltype(*adapter)>::type, TT>::type>,
                  "Only Theory may be applied to a solver builder");
    orderOfTheories.emplace_back(new typename std::remove_reference<decltype(*adapter)>::type::TT());
    return *this;
  }

  SolverBuilder build() const { return SolverBuilder(orderOfTheories); }
};

} // namespace klee

#endif // SOLVERBUILDERFACTORY_H
