#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <unordered_map>
#include <optional>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverBuilder.h"

namespace klee {


class SolverTheory {
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
};

} // namespace klee

#endif