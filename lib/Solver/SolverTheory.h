#ifndef SOLVERTHEORY_H
#define SOLVERTHEORY_H

#include <unordered_map>

#include "klee/ADT/Ref.h"
#include "klee/Expr/Expr.h"

#include "SolverBuilder.h"

namespace klee {

typedef std::vector<ref<ExprHandle>> ArgumentsList;

class SolverAdapter;

struct SolverTheory {
protected:
  ref<SolverAdapter> solverAdapter;

public:
  virtual ref<ExprHandle> sort() = 0;
  virtual ref<ExprHandle> translate(const ref<Expr> &, const ArgumentsList &) = 0;
  virtual ~SolverTheory() = default;
};


struct LIA : public SolverTheory {
protected:
  virtual ref<ExprHandle> translate(const ref<Expr> &, const ArgumentsList &);

public:
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> mul(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> sext(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> zext(const ref<ExprHandle> &lhs,
                               const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> ult(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> ule(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> slt(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sle(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
};

struct FPBV : public SolverTheory {
  virtual ref<ExprHandle> add(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> sub(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> mul(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);
  virtual ref<ExprHandle> div(const ref<ExprHandle> &lhs,
                              const ref<ExprHandle> &rhs);

  virtual ref<ExprHandle> toInt(const ref<ExprHandle> &arg);
  virtual ref<ExprHandle> fromInt(const ref<ExprHandle> &arg);
};

} // namespace klee

#endif