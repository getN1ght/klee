#include "Z3Adapter.h"

#include "SolverBuilder.h"
#include "z3++.h"

using namespace klee;

Z3ExprHandle::Z3ExprHandle(const z3::expr &node) : expr(node) {}

Z3Adapter::Z3Adapter() : ctx(z3::context()) {}

ref<ExprHandle> Z3Adapter::Arrays::read(const ref<ExprHandle> &array,
                                        const ref<ExprHandle> &index) {
  Z3ExprHandle *arrayZ3ExprHandle = dynamic_cast<Z3ExprHandle *>(array.get());
  Z3ExprHandle *indexZ3ExprHandle = dynamic_cast<Z3ExprHandle *>(index.get());

  return (ExprHandle *)(new Z3ExprHandle(
      z3::select(*arrayZ3ExprHandle, *indexZ3ExprHandle)));
}

ref<ExprHandle> Z3Adapter::Arrays::write(const ref<ExprHandle> &array,
                                         const ref<ExprHandle> &index,
                                         const ref<ExprHandle> &value) {
  Z3ExprHandle *arrayZ3ExprHandle = dynamic_cast<Z3ExprHandle *>(array.get());
  Z3ExprHandle *indexZ3ExprHandle = dynamic_cast<Z3ExprHandle *>(index.get());
  Z3ExprHandle *valueZ3ExprHandle = dynamic_cast<Z3ExprHandle *>(value.get());

  return (ExprHandle *)(new Z3ExprHandle(
      z3::store(*arrayZ3ExprHandle, *indexZ3ExprHandle, *valueZ3ExprHandle)));
}

