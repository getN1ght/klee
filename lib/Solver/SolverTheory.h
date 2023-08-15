#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <optional>
#include <unordered_map>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverBuilder.h"

namespace klee {

typedef std::vector<ref<ExprHandle>> ArgumentsList;

struct SolverTheory {
protected:
  virtual ref<ExprHandle> applyHandler(Expr::Kind, const ArgumentsList &) = 0; 

public:
  virtual ref<ExprHandle> translate(const ref<Expr> &expr) {
    std::vector<ref<ExprHandle>> arguments;
    arguments.reserve(expr->getNumKids());
    
    for (const ref<Expr> &kid : expr->kids()) {
      arguments.push_back(translate(kid));
    }

    return applyHandler(expr->getKind(), arguments);
  }

  virtual ~SolverTheory() = default;
};

} // namespace klee

#endif