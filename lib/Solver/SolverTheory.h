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

/*
 * Base class for handles for all builders.
 * Maintains a inner representation for solver
 * and type of expression inside.
 */

class ExprHandle;
typedef std::vector<ref<ExprHandle>> ExprHandleList;

class TheoryHandle {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  enum Kind { COMPLETE, INCOMPLETE };

private:
  const Kind kind;

public:
  const ref<SolverTheory> parent;
  TheoryHandle(Kind kind, const ref<SolverTheory> &parent)
      : kind(kind), parent(parent) {}

  Kind getKind() const { return kind; }
  virtual ~TheoryHandle() {}
};

class CompleteTheoryHandle : public TheoryHandle {
  ref<ExprHandle> handle;

public:
  CompleteTheoryHandle(const ref<ExprHandle> &handle,
                       const ref<SolverTheory> &parent)
      : TheoryHandle(COMPLETE, parent), handle(handle) {}
  ref<ExprHandle> expr() const;

  static bool classof(const TheoryHandle *tr) {
    return tr->getKind() == TheoryHandle::Kind::COMPLETE;
  }
};

class IncompleteResponse : public TheoryHandle {
public:
  typedef std::function<ref<ExprHandle>(const ExprHashMap<ref<ExprHandle>> &)>
      completer_t;

  const completer_t completer;
  const std::vector<ref<Expr>> toBuild;

public:
  IncompleteResponse(const ref<SolverTheory> &parent,
                     const completer_t &completer,
                     const std::vector<ref<Expr>> &required)
      : TheoryHandle(INCOMPLETE, parent), completer(completer),
        toBuild(required) {}

  CompleteTheoryHandle complete(const ExprHashMap<ref<ExprHandle>> &);

  static bool classof(const TheoryHandle *tr) {
    return tr->getKind() == TheoryHandle::Kind::INCOMPLETE;
  }
};

struct SolverTheory {
public:
  enum Sort { UNKNOWN, ARRAYS, BOOL, BV, FPBV, LIA };
  const Sort theorySort = Sort::UNKNOWN;

  typedef ref<ExprHandle> (SolverTheory::*cast_function_t)(
      const ref<ExprHandle> &);

public:
  template <typename ClassName, typename... FArgs, typename... Args>
  ref<TheoryHandle> apply(ref<ExprHandle> (ClassName::*fun)(FArgs...),
                          Args &&...args) {
    std::vector<ref<Expr>> toBuild;
    (
        [&](auto arg) {
          ref<IncompleteResponse> incompleteResponse =
              dyn_cast<IncompleteResponse>(arg);
          if (incompleteResponse == nullptr) {
            return;
          }
          for (const ref<Expr> &raw : incompleteResponse->toBuild) {
            toBuild.push_back(raw);
          }
        }(args),
        ...);

    IncompleteResponse::completer_t completer =
        [&](const ExprHashMap<ref<ExprHandle>> &required) {
          auto unwrap = [&](auto arg) -> ref<ExprHandle> {
            if (ref<IncompleteResponse> incompleteResponse =
                    dyn_cast<IncompleteResponse>(arg)) {
              return incompleteResponse->completer(required);
            } else if (ref<CompleteTheoryHandle> completeResponse =
                           dyn_cast<CompleteTheoryHandle>(arg)) {
              return completeResponse->expr();
            } else {
              // FIXME: handle error properly
              std::abort();
              return nullptr;
            }
          };

          return (reinterpret_cast<ClassName *>(this)->*fun)(unwrap(args)...);
        };

    if (toBuild.empty()) {
      return completer(ExprHashMap<ref<ExprHandle>>());
    } else {
      return new IncompleteResponse(completer, toBuild);
    }
  }

  /* FIXME: figure out better style */
  std::unordered_map<SolverTheory::Sort, cast_function_t> castMapping;

  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  ref<SolverAdapter> solverAdapter;

protected:
  virtual ref<TheoryHandle> castToArray(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToBV(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToBool(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToFPBV(const ref<TheoryHandle> &arg);
  virtual ref<TheoryHandle> castToLIA(const ref<TheoryHandle> &arg);

public:
  SolverTheory(const ref<SolverAdapter> &adapter);

  virtual ref<TheoryHandle> translate(const ref<Expr> &,
                                      const ExprHandleList &) = 0;

  ref<ExprHandle> eq(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs);

  ref<ExprHandle> castTo(Sort sort, const ref<ExprHandle> &arg);

  virtual ~SolverTheory() = default;
};

} // namespace klee

#endif
