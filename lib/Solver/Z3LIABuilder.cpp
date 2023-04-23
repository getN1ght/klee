#include "Z3LIABuilder.h"

#include "z3.h"
#include "llvm/ADT/APInt.h"

using namespace klee;

Z3SortHandle Z3LIABuilder::liaSort() { return {Z3_mk_int_sort(ctx), ctx}; }

Z3ASTHandleLIA Z3LIABuilder::castToSigned(const Z3ASTHandleLIA &expr) {
  if (expr.isSigned) {
    return expr;
  }

  Z3ASTHandleLIA maxSignedValue =
      liaSignedConst(llvm::APInt::getSignedMaxValue(expr.width));

  Z3ASTHandle condition = {Z3_mk_le(ctx, expr, maxSignedValue), ctx};
  const Z3_ast subArgs[] = {expr, maxSignedValue};
  Z3ASTHandleLIA ite = {
      Z3_mk_ite(ctx, condition, expr, Z3_mk_sub(ctx, 2, subArgs)), ctx,
      expr.width, false};

  return {ite, ctx, expr.width, true};
}

Z3ASTHandleLIA Z3LIABuilder::castToUnsigned(const Z3ASTHandleLIA &expr) {
  if (!expr.isSigned) {
    return expr;
  }

  Z3ASTHandleLIA nullValue =
      liaSignedConst(llvm::APInt::getNullValue(expr.width));

  Z3ASTHandle condition = {Z3_mk_lt(ctx, expr, nullValue), ctx};
  Z3ASTHandleLIA maxUnsignedValue =
      liaUnsignedConst(llvm::APInt::getHighBitsSet(expr.width + 1, expr.width));

  const Z3_ast subArgs[] = {maxUnsignedValue, expr};
  Z3ASTHandleLIA ite = {
      Z3_mk_ite(ctx, condition, Z3_mk_sub(ctx, 2, subArgs), expr), ctx,
      expr.width, false};

  return {ite, ctx, expr.width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaUnsignedConst(const llvm::APInt &value) {
  Z3_string s = value.toString(10, true).c_str();
  return {Z3_mk_numeral(ctx, s, liaSort()), ctx, value.getBitWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSignedConst(const llvm::APInt &value) {
  Z3_string s = value.toString(10, false).c_str();
  return {Z3_mk_numeral(ctx, s, liaSort()), ctx, value.getBitWidth(), true};
}

Z3ASTHandleLIA Z3LIABuilder::liaUleExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA unsignedLhs = castToUnsigned(lhs);
  Z3ASTHandleLIA unsignedRhs = castToUnsigned(rhs);
  return {Z3_mk_le(ctx, unsignedLhs, unsignedRhs), ctx, lhs.width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaUltExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_le(ctx, signedLhs, signedRhs), ctx, lhs.width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSleExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_le(ctx, signedLhs, signedRhs), ctx, lhs.width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSltExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_lt(ctx, signedLhs, signedRhs), ctx, lhs.width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaAddExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  const Z3_ast args[] = {lhs, rhs};
  return {Z3_mk_add(ctx, 2, args), ctx, lhs.width, lhs.isSigned};
}

Z3ASTHandleLIA Z3LIABuilder::liaSubExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  const Z3_ast args[] = {lhs, rhs};
  return {Z3_mk_sub(ctx, 2, args), ctx, lhs.width, lhs.isSigned};
}

Z3ASTHandleLIA Z3LIABuilder::liaMulExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  const Z3_ast args[] = {lhs, rhs};
  return {Z3_mk_mul(ctx, 2, args), ctx, lhs.width, lhs.isSigned};
}

Z3ASTHandleLIA Z3LIABuilder::liaUdivExpr(const Z3ASTHandleLIA &lhs,
                                         const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA unsignedLhs = castToUnsigned(lhs);
  Z3ASTHandleLIA unsignedRhs = castToUnsigned(rhs);
  return {Z3_mk_div(ctx, unsignedLhs, unsignedRhs), ctx, lhs.width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSdivExpr(const Z3ASTHandleLIA &lhs,
                                         const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_div(ctx, signedLhs, signedRhs), ctx, lhs.width, true};
}

Z3ASTHandleLIA Z3LIABuilder::liaZextExpr(const Z3ASTHandleLIA &expr, unsigned width) {
  return { expr, ctx, expr.width, expr.isSigned };
}

Z3ASTHandleLIA Z3LIABuilder::liaSextExpr(const Z3ASTHandleLIA &expr, unsigned width) {
  Z3ASTHandleLIA extTerm = liaUnsignedConst(llvm::APInt::getHighBitsSet(width, width - expr.width));
  const Z3_ast terms[] = { expr, extTerm };
  return { Z3_mk_add(ctx, 2, terms), ctx, width, expr.isSigned };
}

/** if *width_out!=1 then result is a bitvector,
    otherwise it is a bool */
Z3ASTHandleLIA Z3LIABuilder::constructActualLIA(ref<Expr> e, int *width_out) {
  int width;
  if (!width_out)
    width_out = &width;

  ++stats::queryConstructs;

  switch (e->getKind()) {
  case Expr::Constant: {
    ConstantExpr *CE = cast<ConstantExpr>(e);
    *width_out = CE->getWidth();

    // Coerce to bool if necessary.
    if (*width_out == 1)
      return CE->isTrue() ? getTrue() : getFalse();

    return liaUnsignedConst(CE->getAPValue());
  }

  // Special
  case Expr::NotOptimized: {
    NotOptimizedExpr *noe = cast<NotOptimizedExpr>(e);
    return construct(noe->src, width_out);
  }

  case Expr::Read: {
    ReadExpr *re = cast<ReadExpr>(e);
    assert(re && re->updates.root);
    *width_out = re->updates.root->getRange();
    return readExpr(getArrayForUpdate(re->updates.root, re->updates.head.get()),
                    construct(re->index, 0));
  }

  case Expr::Select: {
    SelectExpr *se = cast<SelectExpr>(e);
    Z3ASTHandle cond = construct(se->cond, 0);
    Z3ASTHandle tExpr = construct(se->trueExpr, width_out);
    Z3ASTHandle fExpr = construct(se->falseExpr, width_out);
    return iteExpr(cond, tExpr, fExpr);
  }

  case Expr::Concat: {
    ConcatExpr *ce = cast<ConcatExpr>(e);
    unsigned numKids = ce->getNumKids();
    Z3ASTHandle res = construct(ce->getKid(numKids - 1), 0);
    for (int i = numKids - 2; i >= 0; i--) {
      res =
          Z3ASTHandle(Z3_mk_concat(ctx, construct(ce->getKid(i), 0), res), ctx);
    }
    *width_out = ce->getWidth();
    return res;
  }

  case Expr::Extract: {
    ExtractExpr *ee = cast<ExtractExpr>(e);
    Z3ASTHandle src = construct(ee->expr, width_out);
    *width_out = ee->getWidth();
    if (*width_out == 1) {
      return bvBoolExtract(src, ee->offset);
    } else {
      return bvExtract(src, ee->offset + *width_out - 1, ee->offset);
    }
  }

    // Casting

  case Expr::ZExt: {
    int srcWidth;
    CastExpr *ce = cast<CastExpr>(e);
    Z3ASTHandle src = construct(ce->src, &srcWidth);
    *width_out = ce->getWidth();
    if (srcWidth == 1) {
      return iteExpr(src, bvOne(*width_out), bvZero(*width_out));
    } else {
      assert(*width_out > srcWidth && "Invalid width_out");
      return Z3ASTHandle(Z3_mk_concat(ctx, bvZero(*width_out - srcWidth), src),
                         ctx);
    }
  }

  case Expr::SExt: {
    int srcWidth;
    CastExpr *ce = cast<CastExpr>(e);
    Z3ASTHandle src = construct(ce->src, &srcWidth);
    *width_out = ce->getWidth();
    if (srcWidth == 1) {
      return iteExpr(src, bvMinusOne(*width_out), bvZero(*width_out));
    } else {
      return bvSignExtend(src, *width_out);
    }
  }

  // Arithmetic
  case Expr::Add: {
    AddExpr *ae = cast<AddExpr>(e);
    Z3ASTHandle left = construct(ae->left, width_out);
    Z3ASTHandle right = construct(ae->right, width_out);
    assert(*width_out != 1 && "uncanonicalized add");
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvadd(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  case Expr::Sub: {
    SubExpr *se = cast<SubExpr>(e);
    Z3ASTHandle left = construct(se->left, width_out);
    Z3ASTHandle right = construct(se->right, width_out);
    assert(*width_out != 1 && "uncanonicalized sub");
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvsub(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  case Expr::Mul: {
    MulExpr *me = cast<MulExpr>(e);
    Z3ASTHandle right = construct(me->right, width_out);
    assert(*width_out != 1 && "uncanonicalized mul");
    Z3ASTHandle left = construct(me->left, width_out);
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvmul(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  case Expr::UDiv: {
    UDivExpr *de = cast<UDivExpr>(e);
    Z3ASTHandle left = construct(de->left, width_out);
    assert(*width_out != 1 && "uncanonicalized udiv");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(de->right)) {
      if (CE->getWidth() <= 64) {
        uint64_t divisor = CE->getZExtValue();
        if (bits64::isPowerOfTwo(divisor))
          return bvRightShift(left, bits64::indexOfSingleBit(divisor));
      }
    }

    Z3ASTHandle right = construct(de->right, width_out);
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvudiv(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  case Expr::SDiv: {
    SDivExpr *de = cast<SDivExpr>(e);
    Z3ASTHandle left = construct(de->left, width_out);
    assert(*width_out != 1 && "uncanonicalized sdiv");
    Z3ASTHandle right = construct(de->right, width_out);
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvsdiv(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  case Expr::URem: {
    URemExpr *de = cast<URemExpr>(e);
    Z3ASTHandle left = construct(de->left, width_out);
    assert(*width_out != 1 && "uncanonicalized urem");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(de->right)) {
      if (CE->getWidth() <= 64) {
        uint64_t divisor = CE->getZExtValue();

        if (bits64::isPowerOfTwo(divisor)) {
          // FIXME: This should be unsigned but currently needs to be signed to
          // avoid signed-unsigned comparison in assert.
          int bits = bits64::indexOfSingleBit(divisor);

          // special case for modding by 1 or else we bvExtract -1:0
          if (bits == 0) {
            return bvZero(*width_out);
          } else {
            assert(*width_out > bits && "invalid width_out");
            return Z3ASTHandle(Z3_mk_concat(ctx, bvZero(*width_out - bits),
                                            bvExtract(left, bits - 1, 0)),
                               ctx);
          }
        }
      }
    }

    Z3ASTHandle right = construct(de->right, width_out);
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvurem(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  case Expr::SRem: {
    SRemExpr *de = cast<SRemExpr>(e);
    Z3ASTHandle left = construct(de->left, width_out);
    Z3ASTHandle right = construct(de->right, width_out);
    assert(*width_out != 1 && "uncanonicalized srem");
    // LLVM's srem instruction says that the sign follows the dividend
    // (``left``).
    // Z3's C API says ``Z3_mk_bvsrem()`` does this so these seem to match.
    Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvsrem(ctx, left, right), ctx);
    assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
           "width mismatch");
    return result;
  }

  // Bitwise
  case Expr::Not: {
    NotExpr *ne = cast<NotExpr>(e);
    Z3ASTHandle expr = construct(ne->expr, width_out);
    if (*width_out == 1) {
      return notExpr(expr);
    } else {
      return bvNotExpr(expr);
    }
  }

  case Expr::And: {
    AndExpr *ae = cast<AndExpr>(e);
    Z3ASTHandle left = construct(ae->left, width_out);
    Z3ASTHandle right = construct(ae->right, width_out);
    if (*width_out == 1) {
      return andExpr(left, right);
    } else {
      return bvAndExpr(left, right);
    }
  }

  case Expr::Or: {
    OrExpr *oe = cast<OrExpr>(e);
    Z3ASTHandle left = construct(oe->left, width_out);
    Z3ASTHandle right = construct(oe->right, width_out);
    if (*width_out == 1) {
      return orExpr(left, right);
    } else {
      return bvOrExpr(left, right);
    }
  }

  case Expr::Xor: {
    XorExpr *xe = cast<XorExpr>(e);
    Z3ASTHandle left = construct(xe->left, width_out);
    Z3ASTHandle right = construct(xe->right, width_out);

    if (*width_out == 1) {
      // XXX check for most efficient?
      return iteExpr(left, Z3ASTHandle(notExpr(right)), right);
    } else {
      return bvXorExpr(left, right);
    }
  }

  case Expr::Shl: {
    ShlExpr *se = cast<ShlExpr>(e);
    Z3ASTHandle left = construct(se->left, width_out);
    assert(*width_out != 1 && "uncanonicalized shl");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(se->right)) {
      return bvLeftShift(left, (unsigned)CE->getLimitedValue());
    } else {
      int shiftWidth;
      Z3ASTHandle amount = construct(se->right, &shiftWidth);
      return bvVarLeftShift(left, amount);
    }
  }

  case Expr::LShr: {
    LShrExpr *lse = cast<LShrExpr>(e);
    Z3ASTHandle left = construct(lse->left, width_out);
    assert(*width_out != 1 && "uncanonicalized lshr");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(lse->right)) {
      return bvRightShift(left, (unsigned)CE->getLimitedValue());
    } else {
      int shiftWidth;
      Z3ASTHandle amount = construct(lse->right, &shiftWidth);
      return bvVarRightShift(left, amount);
    }
  }

  case Expr::AShr: {
    AShrExpr *ase = cast<AShrExpr>(e);
    Z3ASTHandle left = construct(ase->left, width_out);
    assert(*width_out != 1 && "uncanonicalized ashr");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(ase->right)) {
      unsigned shift = (unsigned)CE->getLimitedValue();
      Z3ASTHandle signedBool = bvBoolExtract(left, *width_out - 1);
      return constructAShrByConstant(left, shift, signedBool);
    } else {
      int shiftWidth;
      Z3ASTHandle amount = construct(ase->right, &shiftWidth);
      return bvVarArithRightShift(left, amount);
    }
  }

    // Comparison

  case Expr::Eq: {
    EqExpr *ee = cast<EqExpr>(e);
    Z3ASTHandle left = construct(ee->left, width_out);
    Z3ASTHandle right = construct(ee->right, width_out);
    if (*width_out == 1) {
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(ee->left)) {
        if (CE->isTrue())
          return right;
        return notExpr(right);
      } else {
        return iffExpr(left, right);
      }
    } else {
      *width_out = 1;
      return eqExpr(left, right);
    }
  }

  case Expr::Ult: {
    UltExpr *ue = cast<UltExpr>(e);
    Z3ASTHandle left = construct(ue->left, width_out);
    Z3ASTHandle right = construct(ue->right, width_out);
    assert(*width_out != 1 && "uncanonicalized ult");
    *width_out = 1;
    return bvLtExpr(left, right);
  }

  case Expr::Ule: {
    UleExpr *ue = cast<UleExpr>(e);
    Z3ASTHandle left = construct(ue->left, width_out);
    Z3ASTHandle right = construct(ue->right, width_out);
    assert(*width_out != 1 && "uncanonicalized ule");
    *width_out = 1;
    return bvLeExpr(left, right);
  }

  case Expr::Slt: {
    SltExpr *se = cast<SltExpr>(e);
    Z3ASTHandle left = construct(se->left, width_out);
    Z3ASTHandle right = construct(se->right, width_out);
    assert(*width_out != 1 && "uncanonicalized slt");
    *width_out = 1;
    return sbvLtExpr(left, right);
  }

  case Expr::Sle: {
    SleExpr *se = cast<SleExpr>(e);
    Z3ASTHandle left = construct(se->left, width_out);
    Z3ASTHandle right = construct(se->right, width_out);
    assert(*width_out != 1 && "uncanonicalized sle");
    *width_out = 1;
    return sbvLeExpr(left, right);
  }

// unused due to canonicalization
#if 0
  case Expr::Ne:
  case Expr::Ugt:
  case Expr::Uge:
  case Expr::Sgt:
  case Expr::Sge:
#endif

  default:
    assert(0 && "unhandled Expr type");
    return getTrue();
  }
}
