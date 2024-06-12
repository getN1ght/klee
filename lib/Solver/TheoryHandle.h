#ifndef THEORYHANDLE_H
#define THEORYHANDLE_H

#include "klee/ADT/Ref.h"

namespace klee {

class SolverHandle;
class Expr;

template <typename T> class TheoryHandle {
public:
  using theory_t = T;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

public:
  const ref<Expr> source;
  const ref<SolverHandle> handle;

  TheoryHandle(const ref<SolverHandle> &handle, const ref<Expr> &source)
      : source(source), handle(handle) {}
};

} // namespace klee

#endif
