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

class TheoryResponse {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  enum Kind { COMPLETE, INCOMPLETE };

private:
  const Kind kind;

public:
  TheoryResponse(Kind kind) : kind(kind) {}

  Kind getKind() const { return kind; }
  virtual ~TheoryResponse() {}
};

class CompleteResponse : public TheoryResponse {
  ref<ExprHandle> handle;

public:
  CompleteResponse(const ref<ExprHandle> &handle)
      : TheoryResponse(COMPLETE), handle(handle) {}
  ref<ExprHandle> expr() const;

  static bool classof(const TheoryResponse *tr) {
    return tr->getKind() == TheoryResponse::Kind::COMPLETE;
  }
};

class IncompleteResponse : public TheoryResponse {
public:
  typedef std::function<ref<ExprHandle>(const ExprHashMap<ref<ExprHandle>> &)>
      completer_t;

  const completer_t completer;
  const std::vector<ref<Expr>> toBuild;

public:
  IncompleteResponse(const completer_t &completer,
                     const std::vector<ref<Expr>> &required)
      : TheoryResponse(INCOMPLETE), completer(completer), toBuild(required) {}

  CompleteResponse complete(const ExprHashMap<ref<ExprHandle>> &);

  static bool classof(const TheoryResponse *tr) {
    return tr->getKind() == TheoryResponse::Kind::INCOMPLETE;
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
  ref<TheoryResponse> apply(ref<ExprHandle> (ClassName::*fun)(FArgs...),
                            Args &&...args) {
    std::vector<ref<Expr>> toBuild;
    (
        [&](auto arg) {
          IncompleteResponse *incompleteResponse =
              dynamic_cast<IncompleteResponse *>(arg.get());
          if (incompleteResponse == nullptr) {
            return;
          }
          for (const ref<Expr> &raw : incompleteResponse->toBuild) {
            toBuild.push_back(raw);
          }
        }(args),
        ...);

    if (toBuild.empty()) {
      auto unwrapCompleteResponse = [&](auto arg) -> ref<ExprHandle> {
        CompleteResponse *completeResponse =
            reinterpret_cast<CompleteResponse *>(arg.get());
        // TODO: assert types. Here we return a ref<ExprHandle>.
        if (completeResponse == nullptr) {
          return arg;
        }
        return completeResponse->expr();
      };

      return new CompleteResponse((reinterpret_cast<ClassName *>(this)->*fun)(
          unwrapCompleteResponse(args)...));
    }

    IncompleteResponse::completer_t completer =
        [&](const ExprHashMap<ref<ExprHandle>> &required) {
          auto unwrap = [&](auto arg) -> ref<ExprHandle> {
            if (IncompleteResponse *incompleteResponse =
                    dynamic_cast<IncompleteResponse *>(arg.get())) {
              return incompleteResponse->completer(required);
            } else if (CompleteResponse *completeResponse =
                           dynamic_cast<CompleteResponse *>(arg.get())) {
              return completeResponse->expr();
            } else {
              return arg;
            }
          };

          return (reinterpret_cast<ClassName *>(this)->*fun)(unwrap(args)...);
        };

    return new IncompleteResponse(completer, toBuild);
  }

  /* FIXME: figure out better style */
  std::unordered_map<SolverTheory::Sort, cast_function_t> castMapping;

  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

protected:
  ref<SolverAdapter> solverAdapter;

protected:
  virtual ref<ExprHandle> castToArray(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> castToBV(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> castToBool(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> castToFPBV(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> castToLIA(const ref<ExprHandle> &arg);

public:
  SolverTheory(const ref<SolverAdapter> &adapter);

  virtual ref<TheoryResponse> translate(const ref<Expr> &,
                                        const ExprHandleList &) = 0;

  ref<ExprHandle> eq(const ref<ExprHandle> &lhs, const ref<ExprHandle> &rhs);

  ref<ExprHandle> castTo(Sort sort, const ref<ExprHandle> &arg);

  virtual ~SolverTheory() = default;
};

class ExprHandle {
public:
  ref<SolverTheory> parent;

  ExprHandle(const ref<SolverTheory> &parent) : parent(parent) {}
public:
  class ReferenceCounter _refCount;

  SolverTheory::Sort sort() const;

  ref<ExprHandle> castTo(SolverTheory::Sort targetSort);

  ExprHandle() = default;
  virtual ~ExprHandle() = default;
};

} // namespace klee

#endif
