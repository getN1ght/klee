#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "SolverTheory.h"
#include "klee/ADT/Ref.h"

#include <unordered_map>
#include <vector>

namespace klee {

class Expr;

class SolverBuilder {
  friend class SolverBuilderFactory;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

private:
  // exprCache cache;
  const std::vector<ref<SolverTheory>> orderOfTheories;

  // Using for optimization
  std::unordered_map<SolverTheory::Sort, uint64_t> positionsOfTheories;

  uint64_t positionOf(SolverTheory::Sort) const;

  SolverBuilder(const std::vector<ref<SolverTheory>> &);
  ref<TheoryHandle> buildWithTheory(const ref<SolverTheory> &theory,
                                    const ref<Expr> &expr);

  ref<TheoryHandle> castToTheory(const ref<TheoryHandle> &arg,
                                 SolverTheory::Sort sort);

public:
  ref<TheoryHandle> build(const ref<Expr> &expr);
};
} // namespace klee

#endif
