#ifndef KLEE_Z3THEORY_H
#define KLEE_Z3THEORY_H

#include "Z3Builder.h"

#include <cstdint>

namespace klee {

struct Z3Theory {
protected:
  Z3_context ctx;

public:
  explicit Z3Theory(Z3_context ctx) : ctx(ctx) {}

  virtual Z3ASTHandle constantExpr(unsigned width, uint64_t value) = 0;

  // Unsigned comparison operations.
  virtual Z3ASTHandle uleExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle ultExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle ugeExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle ugtExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;

  // Signed comparison operations.
  virtual Z3ASTHandle sleExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle sltExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle sgeExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle sgtExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;

  virtual Z3ASTHandle eqExpr(const Z3ASTHandle &lhs,
                             const Z3ASTHandle &rhs) = 0;

  // Bitwise operations.
  virtual Z3ASTHandle andExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle orExpr(const Z3ASTHandle &lhs,
                             const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle xorExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle notExpr(const Z3ASTHandle &expr) = 0;

  // Propositional logic operations.
  virtual Z3ASTHandle iteExpr(const Z3ASTHandle &condition,
                              const Z3ASTHandle &whenTrue,
                              const Z3ASTHandle &whenFalse) = 0;

  // Zero extend operations.
  virtual Z3ASTHandle zextExpr(const Z3ASTHandle &expr, uint64_t width) = 0;
  virtual Z3ASTHandle sextExpr(const Z3ASTHandle &expr, uint64_t width) = 0;

  // Arithmetic operations.
  virtual Z3ASTHandle addExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle subExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle mulExpr(const Z3ASTHandle &lhs,
                              const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle sdivExpr(const Z3ASTHandle &lhs,
                               const Z3ASTHandle &rhs) = 0;
  virtual Z3ASTHandle udivExpr(const Z3ASTHandle &lhs,
                               const Z3ASTHandle &rhs) = 0;

  // Arrays operations.
  virtual Z3ASTHandle readExpr(Z3ASTHandle array, Z3ASTHandle index) = 0;
  virtual Z3ASTHandle writeExpr(Z3ASTHandle array, Z3ASTHandle index,
                                Z3ASTHandle value) = 0;

  virtual Z3ASTHandle extractExpr(Z3ASTHandle array, Z3ASTHandle index) = 0;
  virtual Z3ASTHandle concatExpr(Z3ASTHandle array, Z3ASTHandle index) = 0;
};
} // namespace klee

#endif // KLEE_Z3THEORY_H
