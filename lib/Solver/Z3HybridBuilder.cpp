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

Z3ASTPair Z3HybridBuilder::buildArray(const char *name, unsigned indexWidth,
                                      unsigned rangeWidth) {
  Z3SortHandle domainSortLIA = getLIASort();
  Z3SortHandle domainSortBV = getBvSort(indexWidth);
  Z3SortHandle rangeSort = getBvSort(rangeWidth);

  Z3SortHandle liaArraySort = getArraySort(domainSortLIA, rangeSort);
  Z3SortHandle bvArraySort = getArraySort(domainSortLIA, rangeSort);

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
    Z3ASTHandle un_expr;
    bool hashed = _arr_hash.lookupUpdateNodeExpr(un, un_expr);

    if (!hashed) {
      un_expr = writeExpr(getArrayForUpdate(root, un->next.get()),
                          construct(un->index, 0), construct(un->value, 0));

      _arr_hash.hashUpdateNodeExpr(un, un_expr);
    }

    return (un_expr);
  }
}

Z3ASTPair Z3HybridBuilder::construct(ref<Expr> e) {
  if (!Z3HashConfig::UseConstructHashZ3 || isa<ConstantExpr>(e)) {
    return constructActual(e);
  } else {
    auto liaIterCache = _liaCache.find(e);
    auto bvIterCache = _bvCache.find(e);

    /* I.e. we have already constructed LIA and BV version. */
    if (liaIterCache != _liaCache.end()) {
      return {bvIterCache->second, liaIterCache->second};
    }

    if (bvIterCache != _bvCache.end()) {
      return {bvIterCache->second, Z3ASTHandle()};
    }

    Z3ASTPair res = constructActual(e);
    _bvCache.emplace(res, res.first);
    _liaCache.emplace(res, res.second);
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

    if (width <= 32) {
      return {bvConst32(width, CE->getZExtValue(32)),
              liaUnsignedConstExpr(CE->getAPValue())};
    }
    if (width <= 64) {
      return {bvConst64(width, CE->getZExtValue(64)),
              liaUnsignedConstExpr(CE->getAPValue())};
    }

    /* */
    ref<ConstantExpr> snapCE = CE;
    Z3ASTHandle bvConstResult =
        bvConst64(64, snapCE->Extract(0, 64)->getZExtValue());
    while (snapCE->getWidth() > 64) {
      snapCE = snapCE->Extract(64, snapCE->getWidth() - 64);
      unsigned partialWidth = std::min(64U, snapCE->getWidth());
      bvConstResult = Z3ASTHandle(
          Z3_mk_concat(
              ctx,
              bvConst64(partialWidth,
                        snapCE->Extract(0, partialWidth)->getZExtValue()),
              bvConstResult),
          ctx);
    }
    return Z3ASTPair{bvConstResult, liaUnsignedConstExpr(CE->getAPValue())};
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
      return Z3ASTHandle(Z3_mk_concat(ctx, bvZero(*width_out - srcWidth), src),
                         ctx);
    }
  }

  case Expr::SExt: {
    int srcWidth;
    ref<CastExpr> ce = cast<CastExpr>(e);
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

Z3ASTHandle Z3HybridBuilder::constructActualBV(ref<Expr> e, int *width_out) {
  // pass
}

Z3ASTHandle Z3HybridBuilder::constructActualLIA(ref<Expr> e, int *width_out) {
  // pass
}

Z3HybridBuilder::~Z3HybridBuilder() {}
