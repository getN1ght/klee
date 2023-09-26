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
// #include "FPBV.h"
// #include "LIA.h"
#include "Propositional.h"

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
    return SolverBuilderFactory(ref<SolverAdapter>(new ST()));
  }

  /*
   * Queries a theory into the theories sequence.
   * During builder construction these theories will be applied
   * in order of calls to this method.
   */
  template <typename ST> constexpr SolverBuilderFactory thenApply() noexcept {
    static_assert(std::is_base_of_v<SolverTheory, ST>,
                  "Solver theory required to instantiate a factory");
    orderOfTheories.push_back(new ST(solverAdapter));
    return *this;
  }

  SolverBuilder build() const { return SolverBuilder(orderOfTheories); }
};

} // namespace klee

#endif // SOLVERBUILDERFACTORY_H
