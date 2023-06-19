#include "Z3HybridBuilder.h"

#include "Z3Builder.h"
#include "Z3LIABuilder.h"

#include <string>

using namespace klee;

Z3HybridBuilder::Z3HybridBuilder(bool autoClearConstructCache,
                                 const char *z3LogInteractionFile)
    : Z3Builder(autoClearConstructCache, z3LogInteractionFile) {
  // pass
}

Z3ASTHandle Z3HybridBuilder::bvConst(const llvm::APInt &value) {
    unsigned width = value.getBitWidth();

    /* Fast case. */
    if (width <= 32) {
      return bvConst32(width, value.getZExtValue());
    }
    if (width <= 64) {
      return bvConst64(width, value.getZExtValue());
    }

    /* Slow case. */
    llvm::APInt valueSnap = value;

    Z3ASTHandle bvConstResult =
        bvConst64(64, valueSnap.getLoBits(64).getZExtValue());

    while (valueSnap.getBitWidth() > 64) {
      valueSnap = valueSnap.lshr(llvm::APInt(64, 64));
      unsigned partialWidth = std::min(64U, valueSnap.getBitWidth());
      bvConstResult = Z3ASTHandle( 
          Z3_mk_concat(
              ctx,
              bvConst64(partialWidth,
                        valueSnap.getLoBits(partialWidth).getZExtValue()),
              bvConstResult),
          ctx);
    }

    return bvConstResult;
}

Z3ASTPair Z3HybridBuilder::buildArray(const char *name, unsigned indexWidth,
                                      unsigned rangeWidth) {
  Z3SortHandle domainSortLIA = getLIASort();
  Z3SortHandle domainSortBV = getBvSort(indexWidth);
  Z3SortHandle rangeSort = getBvSort(rangeWidth);

  Z3SortHandle liaArraySort = getArraySort(domainSortLIA, rangeSort);
  Z3SortHandle bvArraySort = getArraySort(domainSortBV, rangeSort);

  Z3_symbol liaSymbol = Z3_mk_string_symbol(
      ctx, const_cast<char *>((std::string(name) + "lia").c_str()));
  Z3_symbol bvSymbol = Z3_mk_string_symbol(
      ctx, const_cast<char *>((std::string(name) + "bv").c_str()));

  return {Z3ASTHandle(Z3_mk_const(ctx, liaSymbol, liaArraySort), ctx),
          Z3ASTHandle(Z3_mk_const(ctx, bvSymbol, bvArraySort), ctx)};
}

Z3ASTPair Z3HybridBuilder::buildConstantArray(const char *name,
                                              unsigned indexWidth,
                                              const llvm::APInt &defaultValue) {
  Z3SortHandle domainSortBV = getBvSort(indexWidth);
  Z3SortHandle domainSortLIA = getLIASort();

  Z3ASTHandle defaultValueAST =
      bvZExtConst(indexWidth, defaultValue.getZExtValue());

  return {
      Z3ASTHandle(Z3_mk_const_array(ctx, domainSortBV, defaultValueAST), ctx),
      Z3ASTHandle(Z3_mk_const_array(ctx, domainSortLIA, defaultValueAST), ctx)};
}

Z3ASTPair Z3HybridBuilder::getInitialArray(const Array *root) {
  assert(root);
  Z3ASTPair result;
  // bool hashed = _arr_hash.lookupArrayExpr(root, array_expr);

  bool hashed = false;

  if (!hashed) {
    // Unique arrays by name, so we make sure the name is unique by
    // using the size of the array hash as a counter.
    std::string uniqueID = llvm::utostr(_arr_hash._array_hash.size());
    std::string uniqueName = root->name + uniqueID;
    if (ref<ConstantWithSymbolicSizeSource> constantWithSymbolicSizeSource =
            dyn_cast<ConstantWithSymbolicSizeSource>(root->source)) {
      result = buildConstantArray(
          uniqueName.c_str(), root->getDomain(),
          llvm::APInt(sizeof(unsigned int) * CHAR_BIT,
                      constantWithSymbolicSizeSource->defaultValue));
    } else {
      result =
          buildArray(uniqueName.c_str(), root->getDomain(), root->getRange());
    }

    if (root->isConstantArray() && constant_array_assertions.count(root) == 0) {
      std::vector<Z3ASTHandle> arrayAssertions;

      for (unsigned i = 0, e = root->constantValues.size(); i != e; ++i) {
        // construct(= (select i root) root->value[i]) to be asserted in
        // Z3Solver.cpp
        Z3ASTPair arrayValuePair = construct(root->constantValues[i]);

        arrayAssertions.push_back(eqExpr(
            readExpr(array_expr, bvConst64(root->getDomain(), i)), arrayValue));
      }
      constant_array_assertions[root] = std::move(arrayAssertions);
    }

    // _arr_hash.hashArrayExpr(root, array_expr);
  }

  return result;
}

Z3ASTPair Z3HybridBuilder::getArrayForUpdate(const Array *root,
                                             const UpdateNode *un) {
  if (!un) {
    return getInitialArray(root);
  } else {
    Z3ASTPair unExprPair;
    // bool hashed = _arr_hash.lookupUpdateNodeExpr(un, unExpr);
    bool hashed = false;

    if (!hashed) {
      Z3ASTPair arrayForUpdatePair = getArrayForUpdate(root, un->next.get());
      Z3ASTPair writeIndexPair = construct(un->index);
      Z3ASTPair writeValuePair = construct(un->index);
      unExprPair = { writeExpr(arrayForUpdatePair.first, writeIndexPair.first, writeValuePair.first),
                     writeExpr(arrayForUpdatePair.second, writeIndexPair.second, writeValuePair.second) };

      // _arr_hash.hashUpdateNodeExpr(un, unExpr);
    }

    return unExprPair;
  }
}

Z3ASTHandle Z3HybridBuilder::bvAdd(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvadd(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvSub(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvsub(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvMul(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvmul(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvUDiv(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvudiv(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvSDiv(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvsdiv(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvAnd(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvadd(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvOr(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvor(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvXor(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvxor(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvNot(const Z3ASTHandle &lhs) {
  return {Z3_mk_bvnot(ctx, lhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvZext(const Z3ASTHandle &expr, unsigned width) {
  return {Z3_mk_zero_ext(ctx, width, expr), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvSext(const Z3ASTHandle &expr, unsigned width) {
  return {Z3_mk_sign_ext(ctx, width, expr), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvExtract(const Z3ASTHandle &expr, unsigned top, unsigned bottom) {
  return {Z3_mk_extract(ctx, top, bottom, expr), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvSrem(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvsrem(ctx, lhs, rhs), ctx};
}

Z3ASTHandle Z3HybridBuilder::bvUrem(const Z3ASTHandle &lhs, const Z3ASTHandle &rhs) {
  return {Z3_mk_bvurem(ctx, lhs, rhs), ctx};
}


Z3ASTPair Z3HybridBuilder::construct(ref<Expr> e) {
  if (!Z3HashConfig::UseConstructHashZ3 || isa<ConstantExpr>(e)) {
    return constructActual(e);
  } else {
    auto iterCache = cache.find(e);

    /* I.e. we have already constructed LIA and BV version. */
    if (iterCache != cache.end()) {
      return iterCache->second;
    }

    Z3ASTPair res = constructActual(e);
    cache.emplace(e, Z3ASTPair{res.first, res.second});
    return res;
  }
}

Z3ASTPair Z3HybridBuilder::constructActual(ref<Expr> e) {
  ++stats::queryConstructs;

  switch (e->getKind()) {
  case Expr::Constant: {
    ref<ConstantExpr> CE = cast<ConstantExpr>(e);
    unsigned width = CE->getWidth();

    // Coerce to bool if necessary.
    if (width == 1) {
      return CE->isTrue() ? Z3ASTPair{getTrue(), getTrue()}
                          : Z3ASTPair{getFalse(), getFalse()};
    }

    return {bvConst(CE->getAPValue()), liaUnsignedConstExpr(CE->getAPValue())};
  }

  // Special
  case Expr::NotOptimized: {
    NotOptimizedExpr *noe = cast<NotOptimizedExpr>(e);
    return construct(noe->src);
  }

  case Expr::Read: {
    ReadExpr *re = cast<ReadExpr>(e);
    assert(re && re->updates.root);

    // While range of array is always a bitvector, then
    // index may be either bitvector or int. Therefore
    // we need to provide pair of array with different
    // sorts.
    Z3ASTPair idxPair = construct(re->index);
    Z3ASTPair arrayPair =
        getArrayForUpdate(re->updates.root, re->updates.head.get());

    return {readExpr(arrayPair.first, idxPair.first),
            readExpr(arrayPair.second, idxPair.second)};
  }

  case Expr::Select: {
    ref<SelectExpr> se = cast<SelectExpr>(e);
    Z3ASTPair condPair = construct(se->cond);
    Z3ASTPair tExprPair = construct(se->trueExpr);
    Z3ASTPair fExprPair = construct(se->falseExpr);
    return {iteExpr(condPair.first, tExprPair.first, fExprPair.first),
            iteExpr(condPair.second, tExprPair.second, fExprPair.second)};
  }

  case Expr::Concat: {
    ref<ConcatExpr> ce = cast<ConcatExpr>(e);
    unsigned numKids = ce->getNumKids();

    Z3ASTPair res = construct(ce->getKid(numKids - 1));

    // Such concat can be constructed only if each
    // part is a bitvector. If not, for LIA we should choose
    // slower case with multiplication or even refuse to construct
    // LIA case.
    for (int i = numKids - 2; i >= 0; i--) {
      Z3ASTPair kidPair = construct(ce->getKid(i));
      res = {{Z3_mk_concat(ctx, kidPair.first, res.first), ctx},
             {Z3_mk_concat(ctx, kidPair.second, res.second), ctx}};
    }
    return res;
  }

  case Expr::Extract: {
    ref<ExtractExpr> ee = cast<ExtractExpr>(e);
    Z3ASTPair src = construct(ee->expr);

    // Extract are totally unsupported in LIA as the require
    // divison by modulo, which is a weak place of Z3 solver
    // in context of LIA.
    if (ee->getWidth() == 1) {
      return {bvBoolExtract(src.first, ee->offset), Z3ASTHandle()};
    } else {
      return {bvExtract(src.first, ee->offset + ee->getWidth() - 1, ee->offset),
              Z3ASTHandle()};
    }
  }

  // Casting
  case Expr::ZExt: {
    ref<CastExpr> ce = cast<CastExpr>(e);
    unsigned srcWidth = ce->src->getWidth();
    unsigned outWidth = ce->getWidth();
    Z3ASTPair src = construct(ce->src);

    // Cast from bool to int in other words.
    if (srcWidth == 1) {
      return {iteExpr(src.first, bvOne(outWidth), bvZero(outWidth)),
              iteExpr(src.second,
                      liaUnsignedConstExpr(llvm::APInt(outWidth, 1)),
                      liaUnsignedConstExpr(llvm::APInt(outWidth, 0)))};
    } else {
      assert(outWidth > srcWidth && "Invalid width_out");
      return { bvZext(src.first, ce->getWidth()), liaZext(src.second, ce->getWidth()) };
    }
  }

  case Expr::SExt: {
    ref<CastExpr> ce = cast<CastExpr>(e);
    int srcWidth = ce->src->getWidth();

    Z3ASTPair src = construct(ce->src);

    unsigned width = ce->getWidth();
    if (srcWidth == 1) {
      return iteExpr(src, bvMinusOne(width), bvZero(width));
    } else {
      return { bvSext(src, width), liaSext(src, width) };
    }
  }

  // Arithmetic
  case Expr::Add: {
    AddExpr *ae = cast<AddExpr>(e);
    Z3ASTPair left = construct(ae->left);
    Z3ASTPair right = construct(ae->right);

    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    return { bvAdd(left.first, right.first),
             liaAdd(left.second, right.second) };
  }

  case Expr::Sub: {
    SubExpr *se = cast<SubExpr>(e);
    Z3ASTPair left = construct(se->left);
    Z3ASTPair right = construct(se->right);
    // assert(*width_out != 1 && "uncanonicalized sub");
    // Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvsub(ctx, left, right), ctx);
    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    return { bvSub(left.first, left.second),
             liaSub(left.second, right.second) };
  }

  case Expr::Mul: {
    MulExpr *me = cast<MulExpr>(e);
    Z3ASTPair left = construct(me->left);
    Z3ASTPair right = construct(me->right);
    // Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvmul(ctx, left, right), ctx);
    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    return { bvMul(left.first, left.second), 
             liaMul(left.second, right.second) };
  }

  case Expr::UDiv: {
    UDivExpr *de = cast<UDivExpr>(e);
    Z3ASTPair left = construct(de->left);

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(de->right)) {
      if (CE->getWidth() <= 64) {
        uint64_t divisor = CE->getZExtValue();
        if (bits64::isPowerOfTwo(divisor))
          return bvRightShift(left, bits64::indexOfSingleBit(divisor));
      }
    }

    Z3ASTPair right = construct(de->right);
    // Z3ASTPair result = Z3ASTHandle(Z3_mk_bvudiv(ctx, left, right), ctx);
    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    return { bvUDiv(left.first, right.first), liaUDiv(left.second, right.second) };
  }

  case Expr::SDiv: {
    ref<SDivExpr> de = cast<SDivExpr>(e);
    Z3ASTPair leftPair = construct(de->left);
    // assert(*width_out != 1 && "uncanonicalized sdiv");
    Z3ASTPair rightPair = construct(de->right);
    
    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    return { bvSDiv(leftPair.first, rightPair.first), 
             liaSDiv(leftPair.second, rightPair.second) };
  }

  case Expr::URem: {
    URemExpr *de = cast<URemExpr>(e);
    Z3ASTPair left = construct(de->left);
    // assert(*width_out != 1 && "uncanonicalized urem");

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

    Z3ASTPair right = construct(de->right);
    // Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvurem(ctx, left, right), ctx);
    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    return { bvUrem(left.first, right.first), liaUrem(left.second, right.second) };
  }

  case Expr::SRem: {
    SRemExpr *de = cast<SRemExpr>(e);
    Z3ASTPair left = construct(de->left);
    Z3ASTPair right = construct(de->right);
    // assert(*width_out != 1 && "uncanonicalized srem");
    // LLVM's srem instruction says that the sign follows the dividend
    // (``left``).
    // Z3's C API says ``Z3_mk_bvsrem()`` does this so these seem to match.
    // Z3ASTHandle result = Z3ASTHandle(Z3_mk_bvsrem(ctx, left, right), ctx);
    // assert(getBVLength(result) == static_cast<unsigned>(*width_out) &&
    //        "width mismatch");
    // return result;
    return { bvSrem(left.first, right.first), liaSrem(left.second, right.second) };
  }

  // Bitwise
  case Expr::Not: {
    NotExpr *ne = cast<NotExpr>(e);
    Z3ASTPair exprPair = construct(ne->expr);
    if (ne->expr->getWidth() == 1) {
      return { notExpr(exprPair.first), notExpr(exprPair.second) };
    } else {
      return { bvNot(exprPair.first), liaNot(exprPair.second) } ;
    }
  }

  case Expr::And: {
    AndExpr *ae = cast<AndExpr>(e);
    Z3ASTPair left = construct(ae->left);
    Z3ASTPair right = construct(ae->right);
    if (ae->getWidth() == 1) {
      return { andExpr(left.first, right.first), andExpr(left.second, right.second) };
    } else {
      return { bvAnd(left.first, right.first), liaAnd(left.second, right.second) };
    }
  }

  case Expr::Or: {
    OrExpr *oe = cast<OrExpr>(e);
    Z3ASTPair left = construct(oe->left);
    Z3ASTPair right = construct(oe->right);
    if (oe->getWidth() == 1) {
      return { orExpr(left.first, right.first), orExpr(left.second, right.second) };
    } else {
      return { bvOr(left.first, right.first), liaOr(left.second, right.second) };
    }
  }

  case Expr::Xor: {
    XorExpr *xe = cast<XorExpr>(e);
    Z3ASTPair left = construct(xe->left);
    Z3ASTPair right = construct(xe->right);

    if (xe->getWidth() == 1) {
      // XXX check for most efficient?
      return iteExpr(left, Z3ASTHandle(notExpr(right)), right);
    } else {
      return { bvXor(left.first, right.first), liaXor(left.second, right.second) };
    }
  }

  case Expr::Shl: {
    ShlExpr *se = cast<ShlExpr>(e);
    Z3ASTPair left = construct(se->left);
    // assert(*width_out != 1 && "uncanonicalized shl");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(se->right)) {
      return bvLeftShift(left, (unsigned)CE->getLimitedValue());
    } else {
      int shiftWidth;
      Z3ASTPair amount = construct(se->right);
      return bvVarLeftShift(left, amount);
    }
  }

  case Expr::LShr: {
    LShrExpr *lse = cast<LShrExpr>(e);
    Z3ASTPair left = construct(lse->left);
    assert(*width_out != 1 && "uncanonicalized lshr");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(lse->right)) {
      return bvRightShift(left, (unsigned)CE->getLimitedValue());
    } else {
      int shiftWidth;
      Z3ASTPair amount = construct(lse->right);
      return bvVarRightShift(left, amount);
    }
  }

  case Expr::AShr: {
    AShrExpr *ase = cast<AShrExpr>(e);
    Z3ASTPair left = construct(ase->left);
    // assert(*width_out != 1 && "uncanonicalized ashr");

    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(ase->right)) {
      unsigned shift = (unsigned)CE->getLimitedValue();
      Z3ASTHandle signedBool = bvBoolExtract(left, *width_out - 1);
      return constructAShrByConstant(left, shift, signedBool);
    } else {
      int shiftWidth;
      Z3ASTPair amount = construct(ase->right, &shiftWidth);
      return bvVarArithRightShift(left, amount);
    }
  }

    // Comparison

  case Expr::Eq: {
    ref<EqExpr> ee = cast<EqExpr>(e);
    Z3ASTPair left = construct(ee->left);
    Z3ASTPair right = construct(ee->right);
    if (ee->left->getWidth() == 1) {
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(ee->left)) {
        if (CE->isTrue()) {
          return right;
        }
        return notExpr(right);
      } else {
        return iffExpr(left, right);
      }
    } else {
      return eqExpr(left, right);
    }
  }

  case Expr::Ult: {
    ref<UltExpr> ue = cast<UltExpr>(e);
    Z3ASTPair left = construct(ue->left);
    Z3ASTPair right = construct(ue->right);
    // assert(*width_out != 1 && "uncanonicalized ult");
    return { bvUlt(left.first, right.first),
             liaUlt(left.second, right.second) };
  }

  case Expr::Ule: {
    ref<UleExpr> ue = cast<UleExpr>(e);
    Z3ASTPair left = construct(ue->left);
    Z3ASTPair right = construct(ue->right);
    // assert(*width_out != 1 && "uncanonicalized ule");
    // *width_out = 1;
    return { bvUle(left.first, right.first),
             liaUle(left.second, right.second) };
  }

  case Expr::Slt: {
    ref<SltExpr> se = cast<SltExpr>(e);
    Z3ASTPair left = construct(se->left);
    Z3ASTPair right = construct(se->right);
    // assert(*width_out != 1 && "uncanonicalized slt");
    // *width_out = 1;
    return { bvSlt(left.first, right.first), liaSlt(left.second, right.second) };
  }

  case Expr::Sle: {
    ref<SleExpr> se = cast<SleExpr>(e);
    Z3ASTPair left = construct(se->left);
    Z3ASTPair right = construct(se->right);
    // assert(*width_out != 1 && "uncanonicalized sle");
    // *width_out = 1;
    return { bvSle(left.first, right.first), liaSle(left.second, right.second) };
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
    return { getTrue(), getTrue() };
  }
}

Z3HybridBuilder::~Z3HybridBuilder() {}
