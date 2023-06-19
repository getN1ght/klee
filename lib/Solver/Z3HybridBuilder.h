#ifndef _KLEE_Z3HYBRIDBUILDER
#define _KLEE_Z3HYBRIDBUILDER

#include "Z3Builder.h"

#include "klee/Expr/ExprHashMap.h"
#include <utility>

namespace klee {

class Z3LIABuilder;
class Z3ASTHandleLIA;

typedef std::pair<Z3ASTHandle, Z3ASTHandleLIA> Z3ASTPair;

class Z3HybridBuilder : public Z3Builder {
private:
  ExprHashMap<Z3ASTPair> cache;

protected:
  Z3ASTPair construct(ref<Expr> e);
  Z3ASTPair constructActual(ref<Expr> e);

  Z3ASTPair buildArray(const char *name, unsigned indexWidth,
                       unsigned rangeWidth);
  Z3ASTPair buildConstantArray(const char *name, unsigned indexWidth,
                               unsigned rangeWidth, const llvm::APInt &defaultValue);

protected:
  /* Propositional logic part */
  Z3ASTHandle propNot(const Z3ASTHandle &expr);
  Z3ASTHandle propAnd(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle propOr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle propXor(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle propIte(const Z3ASTHandle &cond, const Z3ASTHandle &tExpr,
                      const Z3ASTHandle &fExpr);
  Z3ASTHandle propIff(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

protected:
  /* LIA builder part */
  Z3SortHandle getLIASort();

  Z3ASTHandleLIA liaAnd(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaOr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaXor(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaNot(const Z3ASTHandleLIA &expr);

  Z3ASTHandleLIA liaIte(const Z3ASTHandleLIA &condition,
                        const Z3ASTHandleLIA &whenTrue,
                        const Z3ASTHandleLIA &whenFalse);

  Z3ASTHandleLIA liaUnsignedConstExpr(const llvm::APInt &value);
  Z3ASTHandleLIA liaSignedConstExpr(const llvm::APInt &value);

  Z3ASTHandleLIA castToSigned(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA castToUnsigned(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA castToBool(const Z3ASTHandleLIA &expr);

  Z3ASTHandleLIA liaUle(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaUlt(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaSle(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSlt(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaAdd(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSub(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaMul(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSRem(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaURem(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSDiv(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaUDiv(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaZext(const Z3ASTHandleLIA &expr, unsigned width);
  Z3ASTHandleLIA liaSext(const Z3ASTHandleLIA &expr, unsigned width);

protected:
  /* Bitvectors part */
  Z3ASTHandle bvConst(const llvm::APInt &);
  Z3ASTHandle bvAdd(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvSub(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvMul(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvUDiv(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvSDiv(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvAnd(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvOr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvXor(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvNot(const Z3ASTHandle &lhs);

  Z3ASTHandle bvUle(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvUlt(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvSle(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvSlt(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvZext(const Z3ASTHandle &lhs, unsigned width);
  Z3ASTHandle bvSext(const Z3ASTHandle &lhs, unsigned width);
  Z3ASTHandle bvSext(const Z3ASTHandle &lhs, unsigned width);

  Z3ASTHandle bvSrem(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvUrem(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvLShr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvAShr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);
  Z3ASTHandle bvShl(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs);

  Z3ASTHandle bvExtract(const Z3ASTHandle &expr, unsigned top, unsigned bottom);

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