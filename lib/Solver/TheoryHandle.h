#ifndef THEORYHANDLE_H
#define THEORYHANDLE_H

#include "klee/ADT/Ref.h"
#include "klee/Expr/ExprHashMap.h"
#include "klee/util/EDM.h"

#include <functional>
#include <vector>

namespace klee {

class SolverHandle;
template <typename> class SolverTheory;
class Expr;

template <typename T> class TheoryHandle {
public:
  using theory_t = T;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
  enum Kind { COMPLETE, INCOMPLETE, BROKEN };

private:
  const Kind kind;

public:
  const ref<Expr> source;

  TheoryHandle(Kind kind, const ref<Expr> &source)
      : kind(kind), source(source) {}

  Kind getKind() const { return kind; }
  virtual ~TheoryHandle() {}
};

template <typename T> class CompleteTheoryHandle : public TheoryHandle<T> {
  ref<SolverHandle> handle;

public:
  CompleteTheoryHandle(const ref<Expr> &source, const ref<SolverHandle> &handle)
      : TheoryHandle<T>(TheoryHandle<T>::Kind::COMPLETE, source),
        handle(handle) {}

  ref<SolverHandle> expr() const { return handle; }

  template <typename U> static bool classof(const TheoryHandle<U> *tr) {
    return std::is_same_v<T, U> &&
           tr->getKind() == TheoryHandle<T>::Kind::COMPLETE;
  }
};

template <typename T>
class IncompleteResponse
    : public TheoryHandle<T>,
      public Listenable<std::pair<ref<Expr>, ref<TheoryHandle<T>>>> {
public:
  typedef ExprHashMap<ref<TheoryHandle>> TheoryHandleProvider;
  typedef std::function<ref<SolverHandle>(const TheoryHandleProvider &)>
      completer_t;

  const completer_t completer;
  const std::vector<ref<Expr>> toBuild;

public:
  IncompleteResponse(const ref<Expr> &source, const completer_t &completer,
                     const std::vector<ref<Expr>> &required)
      : TheoryHandle(INCOMPLETE, source), completer(completer),
        toBuild(required) {}

  ref<CompleteTheoryHandle<T>> complete(const TheoryHandleProvider &);

  template <typename U> static bool classof(const TheoryHandle<U> *tr) {
    return std::is_same_v<T, U> &&
           tr->getKind() == TheoryHandle<T>::Kind::INCOMPLETE;
  }
};

/*
 * Reports that the parent theory can not construct
 * handle for a given expression.
 */
template <typename T> class BrokenTheoryHandle : public TheoryHandle<T> {
public:
  BrokenTheoryHandle(const ref<Expr> &source) : TheoryHandle(BROKEN, source) {}

  template <typename U> static bool classof(const TheoryHandle<U> *tr) {
    return std::is_same_v<T, U> &&
           tr->getKind() == TheoryHandle<T>::Kind::BROKEN;
  }
};
} // namespace klee

#endif
