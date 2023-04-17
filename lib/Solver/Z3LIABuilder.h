#ifndef Z3_LIA_BUILDER_H
#define Z3_LIA_BUILDER_H

#include "Z3Builder.h"

namespace klee {

/**
 * Additionally maintain width of integer under
 * ASTNode and it's signness.
 */
class Z3ASTLIAHandle : public Z3ASTHandle {
private:
  bool isSigned = false;
  unsigned width;

public:
  Z3ASTLIAHandle(unsigned _width, bool _isSigned)
      : width(_width), isSigned(_isSigned) {}

  Z3ASTLIAHandle castToSigned() const {
    if (isSigned) {
      return *this;
    }
    Z3ASTLIAHandle castedNode(width, !isSigned);
    llvm::report_fatal_error("Not implemented!");
    return castedNode;
  }

  Z3ASTLIAHandle castToUnsigned() const {
    if (!isSigned) {
      return *this;
    }
    Z3ASTLIAHandle castedNode(width, !isSigned);
    llvm::report_fatal_error("Not implemented!");
    return castedNode;
  }
};

class Z3LIABuilder : public Z3Builder {
protected:
  Z3SortHandle getIntSort();
  virtual Z3ASTLIAHandle buildArray(const char *name, unsigned indexWidth,
                                    unsigned valueWidth);

public:
  // Linear integer arithmetics comparison operations
  Z3ASTLIAHandle uge(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle ule(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle ult(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle ugt(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);

  Z3ASTLIAHandle sge(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle sle(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle slt(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle sgt(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);

  Z3ASTLIAHandle eq(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);

  // Linear integer arithmetics operations
  Z3ASTLIAHandle add(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle sub(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle mult(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle div(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);

  // Arithmetic shift operations.
  // Represents as division or multiplication with power of 2.
  Z3ASTLIAHandle lshr(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle ashr(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle shl(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);

  // Changes type of ast handle.
  Z3ASTLIAHandle zext(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
  Z3ASTLIAHandle sext(Z3ASTLIAHandle lhs, Z3ASTLIAHandle rhs);
};
} // namespace klee

#endif