#ifndef SOLVERBUILDERFACTORY_H
#define SOLVERBUILDERFACTORY_H

#include <memory>
#include <type_traits>
#include <vector>

#include "SolverAdapter.h"
#include "SolverBuilder.h"
#include "SolverTheory.h"

#include "Arrays.h"
#include "BV.h"
// #include "LIA"

namespace klee {  
class SolverBuilderFactory {
private:
  ref<SolverAdapter> solverAdapter;
  std::vector<ref<SolverTheory>> orderOfTheories;

  SolverBuilderFactory(const ref<SolverAdapter> &solverAdapter)
      : solverAdapter(solverAdapter) {}

public:
  /*
   * Constructs the factory for specified solver.
   * Takes class of adapter.
   */
  template<typename ST>
  static SolverBuilderFactory forSolver() {
    static_assert(std::is_base_of_v<SolverAdapter, ST>,
                  "Solver adapter required to instantiate a factory");
    SolverBuilderFactory solverBuilderFactory(ref<SolverAdapter>(new ST()));
    return solverBuilderFactory;
  }

  /*
   * Queries a theory into the theories sequence.
   * During builder construction these theories will be applied
   * in order of calls to this method.
   */
  template <SolverTheory::Sort S> SolverBuilderFactory thenApply();
  // template <> SolverBuilderFactory thenApply<SolverTheory::Sort::LIA>() {
  //   orderOfTheories.push_back(ref<SolverTheory>(new LIA()));
  // }


  SolverBuilder build() const { return SolverBuilder(orderOfTheories); }
};

template <>
SolverBuilderFactory
SolverBuilderFactory::thenApply<SolverTheory::Sort::ARRAYS>() {
  orderOfTheories.push_back(ref<SolverTheory>(new Arrays(solverAdapter)));
}
// FIXME: we should not select width on this stage
template <>
SolverBuilderFactory SolverBuilderFactory::thenApply<SolverTheory::Sort::BV>() {
  orderOfTheories.push_back(ref<SolverTheory>(new BV(solverAdapter)));
}

} // namespace klee

#endif // SOLVERBUILDERFACTORY_H
