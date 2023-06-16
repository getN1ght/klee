#ifndef _KLEE_Z3HYBRIDBUILDER
#define _KLEE_Z3HYBRIDBUILDER

#include "Z3Builder.h"

#include "klee/Expr/ExprHashMap.h"
#include <utility>

namespace klee {

class Z3LIABuilder;

typedef std::pair<Z3ASTHandle, Z3ASTHandle> Z3ASTPair;

class Z3HybridBuilder : public Z3Builder {
private:
  ExprHashMap<Z3ASTHandle> _bvCache;
  ExprHashMap<Z3ASTHandle> _liaCache;

  Z3ASTHandle constructActualBV(ref<Expr> e, int *width_out);
  Z3ASTHandle constructActualLIA(ref<Expr> e, int *width_out);

protected:
  Z3ASTPair construct(ref<Expr> e);
  Z3ASTPair constructActual(ref<Expr> e);

  Z3ASTPair buildArray(const char *name, unsigned indexWidth,
                       unsigned rangeWidth);
  Z3ASTPair buildConstantArray(const char *name, unsigned indexWidth,
                               const llvm::APInt &defaultValue);

protected:
  /* LIA builder part */
  Z3SortHandle getLIASort();

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

protected:
  /* Bitvectors part */

private:
  Z3ASTPair getArrayForUpdate(const Array *root, const UpdateNode *un);
  Z3ASTPair getInitialArray(const Array *root);

public:
  explicit Z3HybridBuilder(bool autoClearConstructCache,
                           const char *z3LogInteractionFile);
  ~Z3HybridBuilder() override;
};

} // namespace klee

#endif // _KLEE_Z3HYBRIDBUILDER