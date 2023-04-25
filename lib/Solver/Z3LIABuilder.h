#ifndef KLEE_Z3LIABUILDER_H
#define KLEE_Z3LIABUILDER_H

#include "Z3Builder.h"

namespace llvm {
  class APInt;
}


namespace klee {

class Z3ASTHandleLIA : public Z3ASTHandle {
private:
  unsigned width;
  bool isSigned;

public:
  Z3ASTHandleLIA() = default;
  Z3ASTHandleLIA(const Z3_ast &node, const Z3_context &ctx, unsigned _width,
                 bool _isSigned)
      : Z3ASTHandle(node, ctx), width(_width), isSigned(_isSigned) {}

  unsigned getWidth() const {
    return width;
  }
  bool sign() const {
    return isSigned;
  }
};

class Z3ArrayExprLIAHash : public ArrayExprHash<Z3ASTHandleLIA> {
  friend class Z3LIABuilder;

public:
  Z3ArrayExprLIAHash() = default;
  ~Z3ArrayExprLIAHash() override = default;

  void clear() {
    _update_node_hash.clear();
    _array_hash.clear();
  }
  void clearUpdates() { _update_node_hash.clear(); }
};

class Z3LIABuilder : public Z3Builder {
private:
  ExprHashMap<Z3ASTHandleLIA> constructedLIA;
  Z3ArrayExprLIAHash arrHashLIA;

  Z3SortHandle liaSort();

  Z3ASTHandleLIA liaAnd(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaOr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaXor(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);
  Z3ASTHandleLIA liaNot(const Z3ASTHandleLIA &expr);

  Z3ASTHandleLIA liaIte(const Z3ASTHandleLIA &condition,
                        const Z3ASTHandleLIA &whenTrue,
                        const Z3ASTHandleLIA &whenFalse);

  Z3ASTHandleLIA liaEq(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  Z3ASTHandleLIA handleUnsignedOverflow(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA handleUnsignedUnderflow(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA handleSignedOverflow(const Z3ASTHandleLIA &expr);
  Z3ASTHandleLIA handleSignedUnderflow(const Z3ASTHandleLIA &expr);

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

  Z3ASTHandleLIA liaConcatExpr(const Z3ASTHandleLIA &lhs, const Z3ASTHandleLIA &rhs);

  // Array operations
  Z3ASTHandleLIA liaWriteExpr(const Z3ASTHandleLIA &array, const Z3ASTHandleLIA &index,
                              const Z3ASTHandleLIA &value);
  Z3ASTHandleLIA liaReadExpr(const Z3ASTHandleLIA &array, const Z3ASTHandleLIA &index);

  Z3ASTHandleLIA liaGetInitialArray(const Array *os);
  Z3ASTHandleLIA liaGetArrayForUpdate(const Array *root, const UpdateNode *un);
  Z3ASTHandleLIA liaBuildArray(const char *name, unsigned width);
  Z3ASTHandleLIA liaBuildConstantArray(const char *name, const llvm::APInt &defaultValue);

  Z3ASTHandleLIA constructLIA(const ref<Expr> &e);
  Z3ASTHandleLIA constructActualLIA(const ref<Expr> &e);
public:

  Z3ASTHandle construct(ref<Expr> e, int *width_out) override;
};
}
#endif // KLEE_Z3LIABUILDER_H
