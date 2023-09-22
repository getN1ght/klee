#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "llvm/Support/Casting.h"

#include <type_traits>
#include <unordered_map>

namespace klee {

class SolverAdapter;
template <typename> class ExprHashMap;



/* TODO: implement event-driven model. */
class Notifiable;

class Listenable {
private:
  std::vector<ref<Notifiable>> listeners;
public:
  void listen(const ref<Notifiable> &listener) {
    listeners.push_back(listener);
  }
  void notifyAll() {
    for (const auto &listener : listeners) {
      listener->notify();
    }
  }
};

class Notifiable {
  friend class Listenable;
protected:
  virtual void notify() {}
};


/*
 * Base class for handles for all builders.
 * Maintains a inner representation for solver
 * and type of expression inside.
 */

struct SolverTheory;

class SolverHandle;

class TheoryHandle {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
  enum Kind { COMPLETE, INCOMPLETE, BROKEN };

private:
  const Kind kind;

public:
  // TODO: implement
  const ref<Expr> source = nullptr;
  
  // TODO: should we have a reference to a parent theory?
  ref<SolverTheory> parent;
  TheoryHandle(Kind kind, const ref<SolverTheory> &parent)
      : kind(kind), parent(parent) {}

  Kind getKind() const { return kind; }
  virtual ~TheoryHandle() {}
};

typedef std::vector<ref<TheoryHandle>> TheoryHandleList;

class CompleteTheoryHandle : public TheoryHandle {
  ref<SolverHandle> handle;

public:
  CompleteTheoryHandle(const ref<SolverHandle> &handle,
                       const ref<SolverTheory> &parent)
      : TheoryHandle(COMPLETE, parent), handle(handle) {}
  ref<SolverHandle> expr() const;

  static bool classof(const TheoryHandle *tr) {
    return tr->getKind() == TheoryHandle::Kind::COMPLETE;
  }
};

class IncompleteResponse : public TheoryHandle {
public:
  typedef ExprHashMap<ref<TheoryHandle>> TheoryHandleProvider;
  typedef std::function<ref<SolverHandle>(const TheoryHandleProvider &)>
      completer_t;

  const completer_t completer;
  const std::vector<ref<Expr>> toBuild;

public:
  IncompleteResponse(const ref<SolverTheory> &parent,
                     const completer_t &completer,
                     const std::vector<ref<Expr>> &required)
      : TheoryHandle(INCOMPLETE, parent), completer(completer),
        toBuild(required) {}

  CompleteTheoryHandle complete(const TheoryHandleProvider &);

  static bool classof(const TheoryHandle *tr) {
    return tr->getKind() == TheoryHandle::Kind::INCOMPLETE;
  }
};

/* 
 * Reports that the parent theory can not construct
 * handle for a given expression.
 */
class BrokenTheoryHandle : public TheoryHandle {
private:
  ref<Expr> expr;

public:
  ref<Expr> cause() const { return expr; }

  BrokenTheoryHandle(const ref<Expr> &expr)
      : TheoryHandle(BROKEN, nullptr), expr(expr) {}

  static bool classof(const TheoryHandle *tr) {
    return tr->getKind() == TheoryHandle::Kind::BROKEN;
  }
};

struct SolverTheory {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

public:
  enum Sort { ARRAYS, BOOL, BV, FPBV, LIA };
  const Sort theorySort;

  typedef ref<TheoryHandle> (SolverTheory::*cast_function_t)(
      const ref<TheoryHandle> &);

public:
  template <typename Functor, typename... Args>
  ref<TheoryHandle> apply(const Functor &functor, Args &&...args) {
    std::vector<ref<Expr>> toBuild;
    bool allConstructable = ([&](auto arg) -> bool {
      if (isa<BrokenTheoryHandle>(arg)) {
        return false;
      }

      if (ref<IncompleteResponse> incompleteResponse =
              dyn_cast<IncompleteResponse>(arg)) {
        for (const ref<Expr> &raw : incompleteResponse->toBuild) {
          toBuild.push_back(raw);
        }
      }
      return true;
    }(args) && ...);

    if (!allConstructable) {
      // FIXME: report with correct expression.
      return new BrokenTheoryHandle(Expr::createFalse());
    }

    IncompleteResponse::completer_t completer =
        [functor,
         args...](const IncompleteResponse::TheoryHandleProvider &required)
        -> ref<SolverHandle> {
      auto unwrap = [&required](auto arg) -> ref<SolverHandle> {
        if (ref<IncompleteResponse> incompleteTheoryHandle =
                dyn_cast<IncompleteResponse>(arg)) {
          arg = incompleteTheoryHandle->completer(required);
        }
        if (ref<CompleteTheoryHandle> completeTheoryHandle =
                dyn_cast<CompleteTheoryHandle>(arg)) {
          return completeTheoryHandle->expr();
        }
        std::abort();
        return nullptr;
      };

      return functor(unwrap(args)...);
    };

    if (toBuild.empty()) {
      return new CompleteTheoryHandle(
          completer(IncompleteResponse::TheoryHandleProvider()), this);
    } else {
      return new IncompleteResponse(this, completer, toBuild);
    }
  }

  /* FIXME: figure out better style */
  std::unordered_map<SolverTheory::Sort, cast_function_t> castMapping;

protected:
  ref<SolverAdapter> solverAdapter;

protected:
  virtual ref<TheoryHandle> castToArray(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToBV(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToBool(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToFPBV(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToLIA(const ref<TheoryHandle> &arg);

public:
  SolverTheory(Sort, const ref<SolverAdapter> &);

  virtual ref<TheoryHandle> translate(const ref<Expr> &,
                                      const TheoryHandleList &) = 0;

  ref<TheoryHandle> castTo(Sort sort, const ref<TheoryHandle> &arg);

  Sort getSort() const { return theorySort; }

  virtual ~SolverTheory() = default;
};

} // namespace klee

#endif
