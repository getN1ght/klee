#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "SolverTheory.h"

#include "klee/ADT/BiMap.h"
#include "klee/ADT/Ref.h"

#include "klee/Expr/ExprHashMap.h"

#include "klee/util/EDM.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace klee {

class Expr;

class SolverBuilder {
  friend class SolverBuilderFactory;
  typedef ExprHashMap<ref<TheoryHandle<>>> cache_t;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  /* Listens to cache just completed Incomplete Handles */
  // void
  // onNotify(const std::pair<ref<Expr>, ref<TheoryHandle<T>>> &completed) override;

private:
  /* Cache for already built expressions */
  cache_t cache;

  /* Array of theories which may be applied in this builder */
  BiArray<ref<SolverTheory>> orderOfTheories;

  SolverBuilder(const std::vector<ref<SolverTheory>> &);

  template <typename RT, typename AT>
  ref<TheoryHandle<RT>> buildWithTheory(const ref<SolverTheory<AT>> &theory,
                                        const ref<Expr> &expr);

  // ref<TheoryHandle> castToTheory(const ref<TheoryHandle> &arg,
  //                                SolverTheory::Sort sort);

public:
  template <typename RT> ref<TheoryHandle<RT>> build(const ref<Expr> &expr);
};
} // namespace klee

#endif
