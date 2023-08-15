#ifndef Z3ADAPTER_H
#define Z3ADAPTER_H

#include "SolverTheory.h"
#include "SolverAdapter.h"

#include "z3++.h"

namespace klee {

class Z3ExprHandle : public ExprHandle {
private:
  const z3::expr expr;

public:
  Z3ExprHandle(const z3::expr &);

  operator const z3::expr &() const { return expr; }
};

class Z3Adapter : public SolverAdapter {
private:
  const z3::context ctx;

public:
  Z3Adapter();

  ~Z3Adapter() = default;
};

};

#endif