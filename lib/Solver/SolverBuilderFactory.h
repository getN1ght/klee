#ifndef SOLVERBUILDERFACTORY_H
#define SOLVERBUILDERFACTORY_H

#include <type_traits>

#include "SolverAdapter.h"
#include "SolverBuilder.h"
#include "SolverTheory.h"

// #include "Arrays.h"
// #include "BV.h"
// #include "FPBV.h"
// #include "LIA.h"
// #include "Propositional.h"

namespace klee {
class SolverBuilderFactory {
private:
  template <typename... Theory> struct RawSolverBuilder {
    const ref<SolverAdapter> solverAdapter;

    constexpr RawSolverBuilder(const ref<SolverAdapter> &solverAdapter)
        : solverAdapter(solverAdapter) {}

    RawSolverBuilder(const RawSolverBuilder &) = delete;
    RawSolverBuilder &operator=(const RawSolverBuilder &) = delete;

    template <typename... ATheory>
    constexpr RawSolverBuilder(RawSolverBuilder<ATheory...> &&another)
        : solverAdapter(another.solverAdapter) {}

    /*
     * Queries a theory into the theories sequence.
     * During builder construction these theories will be applied
     * in order of calls to this method.
     */
    template <typename ST>
    constexpr RawSolverBuilder<Theory..., SolverTheory<ST>>
    thenApply() noexcept {
      return RawSolverBuilder<Theory..., SolverTheory<ST>>(std::move(*this));
    }

    constexpr SolverBuilder<Theory...> build() const {
      return SolverBuilder<Theory...>();
    }
  };

public:
  /*
   * Constructs the factory for specified solver.
   * Takes class of adapter.
   */
  template <typename SA> static RawSolverBuilder<> forSolver() {
    static_assert(std::is_base_of_v<SolverAdapter, SA>,
                  "Solver adapter required to instantiate a factory");
    return RawSolverBuilder<>(new SA());
  }
};

} // namespace klee

#endif // SOLVERBUILDERFACTORY_H
