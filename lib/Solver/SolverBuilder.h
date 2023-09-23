#ifndef SOLVERBUILDER_H
#define SOLVERBUILDER_H

#include "SolverTheory.h"

#include "klee/ADT/Ref.h"
#include "klee/ADT/BiMap.h"

#include "klee/Expr/ExprHashMap.h"

#include "klee/util/EDM.h"

#include <unordered_map>
#include <vector>
#include <functional>


namespace klee {

class Expr;

class SolverBuilder : public Notifiable<std::pair<ref<Expr>, ref<TheoryHandle>>> {
  friend class SolverBuilderFactory;
  typedef ExprHashMap<ref<TheoryHandle>> cache_t;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  void onNotify(const std::pair<ref<Expr>, ref<TheoryHandle>> &completed) override;

private:
  /* Cache for already built expressions. */
  cache_t cache;

  BiArray<ref<SolverTheory>> orderOfTheories;

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
