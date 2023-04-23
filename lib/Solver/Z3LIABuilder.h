#ifndef KLEE_Z3LIABUILDER_H
#define KLEE_Z3LIABUILDER_H

#include "Z3Builder.h"

namespace llvm {
  class APInt;
}


namespace klee {

class Z3ASTHandleLIA : public Z3ASTHandle {
public:
  const unsigned width;
  const bool isSigned;

  Z3ASTHandleLIA(const Z3_ast &node, const Z3_context &ctx, unsigned _width,
                 bool _isSigned)
      : Z3ASTHandle(node, ctx), width(_width), isSigned(_isSigned) {}
};

class Z3LIABuilder : public Z3Builder {
private:
  Z3SortHandle liaSort();

  Z3ASTHandleLIA liaUnsignedConst(const llvm::APInt &value);
  Z3ASTHandleLIA liaSignedConst(const llvm::APInt &value);

  Z3ASTHandleLIA castToSigned(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA castToUnsigned(const Z3ASTHandleLIA &expr);

  Z3ASTHandleLIA liaUleExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaUltExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaSleExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSltExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaAddExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSubExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaMulExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaUdivExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaSdivExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA liaZextExpr(const Z3ASTHandleLIA &expr, unsigned width);
  Z3ASTHandleLIA liaSextExpr(const Z3ASTHandleLIA &expr, unsigned width);

public:
  Z3ASTHandleLIA constructActualLIA(ref<Expr> e, int *width_out);
};
}
#endif // KLEE_Z3LIABUILDER_H
