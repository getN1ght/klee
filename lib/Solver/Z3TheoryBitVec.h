#ifndef KLEE_Z3THEORYBITVEC_H
#define KLEE_Z3THEORYBITVEC_H

#include "Z3Theory.h"

namespace klee {

struct Z3TheoryBitVec : public Z3Theory {
private:
  Z3SortHandle bvSort(unsigned width);

public:
  Z3ASTHandle constantExpr(unsigned int width, uint64_t value) override;

  // Unsigned comparison operations.
  Z3ASTHandle uleExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle ultExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle ugeExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle ugtExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;

  // Signed comparison operations.
  Z3ASTHandle sleExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle sltExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle sgeExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle sgtExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;

  Z3ASTHandle eqExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;

  // Bitwise operations.
  Z3ASTHandle andExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle orExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle xorExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle notExpr(const Z3ASTHandle &expr) override;

  // Propositional logic operations.
  Z3ASTHandle iteExpr(const Z3ASTHandle &condition, const Z3ASTHandle &whenTrue,
                      const Z3ASTHandle &whenFalse) override;

  // Zero extend operations.
  Z3ASTHandle zextExpr(const Z3ASTHandle &expr, uint64_t width) override;
  Z3ASTHandle sextExpr(const Z3ASTHandle &expr, uint64_t width) override;

  // Arithmetic operations.
  Z3ASTHandle addExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle subExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle mulExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle sdivExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
  Z3ASTHandle udivExpr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) override;
};

} // namespace klee

#endif // KLEE_Z3THEORYBITVEC_H
