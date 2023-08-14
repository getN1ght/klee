#include "Z3Adapter.h"

#include "SolverBuilder.h"
#include "z3++.h"

using namespace klee;

Z3ExprHandle::Z3ExprHandle(const z3::expr &node) : expr(node) {}

Z3Adapter::Z3Adapter() : ctx(z3::context()) {}

// ExprHandle Z3Adapter::Arrays::read(const ExprHandle &array,
//                                    const ExprHandle &index) {
//   dynamic_cast<const Z3ExprHandle &>(array)
// }

// ExprHandle Z3Adapter::Arrays::write(const ExprHandle &array,
//                                     const ExprHandle &index,
//                                     const ExprHandle &value) {

//   //
// }


// ExprHandle Z3Adapter::BV::add(const ExprHandle &lhs, const ExprHandle &rhs) {}
