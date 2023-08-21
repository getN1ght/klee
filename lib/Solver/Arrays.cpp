#include "SolverAdapter.h"

#include <klee/ADT/Ref.h>

using namespace klee;

ref<ExprHandle> Arrays::translate(Expr::Kind kind,
                                     const ArgumentsList &args) {
  switch (kind) {
  case Expr::Kind::Read: {
    return read(args[0], args[1]);
  }
  default: {
    return nullptr;
  }
  }
}

ref<ExprHandle> Arrays::read(const ref<ExprHandle> &arr,
                             const ref<ExprHandle> &idx) {
  return solverAdapter->read(arr, idx);
}

ref<ExprHandle> Arrays::write(const ref<ExprHandle> &arr,
                              const ref<ExprHandle> &idx,
                              const ref<ExprHandle> &val) {
  return solverAdapter->write(arr, idx, val);
}
