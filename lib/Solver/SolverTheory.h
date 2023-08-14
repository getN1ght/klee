#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <optional>
#include <unordered_map>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverBuilder.h"

namespace klee {

template<typename Fn, typename...Args>
class Handler {
private:
  Fn function;

public:
  Handler(Fn function) : function(function) {}

  ExprHandle apply(Args&&...args) {
    function(args...);
  }
};

struct SolverTheory {
public:
  enum Kind {
    ARR,
    BV,
    LIA,
  };

protected:
  std::unordered_map<Expr::Kind, int> handlers;

public:
  std::optional<ExprHandle> translate(const ref<Expr> &expr) {
    Expr::Kind exprKind = expr->getKind();

    if (handlers.count(exprKind)) {
      return std::optional<ExprHandle>();
    }

    handlers.at(exprKind);
    return std::optional<ExprHandle>();
  }

  virtual ~SolverTheory() = default;
};

} // namespace klee

#endif