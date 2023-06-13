#ifndef _KLEE_Z3HYBRIDBUILDER
#define _KLEE_Z3HYBRIDBUILDER

#include "Z3Builder.h"

#include "klee/Expr/ExprHashMap.h"
#include <utility>

namespace klee {

class Z3LIABuilder;

class Z3HybridBuilder : public Z3Builder {
private:
  ExprHashMap<std::pair<Z3ASTHandle, int>> _bvCache;
  ExprHashMap<std::pair<Z3ASTHandle, int>> _liaCache;

  Z3ASTHandle constructActualBV(ref<Expr> e, int *width_out);
  Z3ASTHandle constructActualLIA(ref<Expr> e, int *width_out);

protected:
  Z3ASTHandle construct(ref<Expr> e, int *width_out) override;
  Z3ASTHandle constructActual(ref<Expr> e, int *width_out) override;

protected:
  /* LIA builder part */
  Z3SortHandle liaSort();

  Z3ASTHandleLIA liaAndExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaOrExpr(const Z3ASTHandleLIA &lhs,
                           const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaXorExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaNotExpr(const Z3ASTHandleLIA &expr);

  Z3ASTHandleLIA liaIteExpr(const Z3ASTHandleLIA &condition,
                            const Z3ASTHandleLIA &whenTrue,
                            const Z3ASTHandleLIA &whenFalse);

  Z3ASTHandleLIA liaEqExpr(const Z3ASTHandleLIA &lhs,
                           const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaUnsignedConstExpr(const llvm::APInt &value);
  Z3ASTHandleLIA liaSignedConstExpr(const llvm::APInt &value);

  Z3ASTHandleLIA castToSigned(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA castToUnsigned(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA castToBool(const Z3ASTHandleLIA &expr);

  Z3ASTHandleLIA liaUleExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaUltExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaSleExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSltExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaAddExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSubExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaMulExpr(const Z3ASTHandleLIA &lhs,
                            const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaZextExpr(const Z3ASTHandleLIA &expr, unsigned width);
  Z3ASTHandleLIA liaSextExpr(const Z3ASTHandleLIA &expr, unsigned width);

public:
  explicit Z3HybridBuilder(bool autoClearConstructCache,
                           const char *z3LogInteractionFile);
  ~Z3HybridBuilder() override;
};

} // namespace klee

#endif // _KLEE_Z3HYBRIDBUILDER