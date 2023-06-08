#include "Z3LIABuilder.h"

#include "z3.h"
#include "llvm/ADT/APInt.h"

#include "klee/Expr/ExprUtil.h"

using namespace klee;

Z3SortHandle Z3LIABuilder::liaSort() { return {Z3_mk_int_sort(ctx), ctx}; }

Z3ASTHandleLIA
Z3LIABuilder::handleUnsignedOverflow(const Z3ASTHandleLIA &expr) {
  assert(!expr.sign());

  // Unsigned overflow
  // assumes: expr \in [-(2**w)+1, 2**(w+1)-2]
  // gives:   sum = (a + b >= 2**w) ? (a + b - 2**w) : (a + b);
  Z3ASTHandleLIA maxUnsignedInt =
      liaUnsignedConst(llvm::APInt::getHighBitsSet(expr.getWidth() + 1, 1));
  Z3ASTHandleLIA condition = {Z3_mk_ge(ctx, expr, maxUnsignedInt), ctx, 1,
                              false};

  Z3_ast subArgs[] = {expr, maxUnsignedInt};
  Z3ASTHandleLIA subHandleLIA = {Z3_mk_sub(ctx, 2, subArgs), ctx,
                                 expr.getWidth(), false};
  return {Z3_mk_ite(ctx, condition, subHandleLIA, expr), ctx, expr.getWidth(),
          false};
}

Z3ASTHandleLIA
Z3LIABuilder::handleUnsignedUnderflow(const Z3ASTHandleLIA &expr) {
  assert(!expr.sign());

  // Unsigned underflow
  // assumes: expr \in [-(2**w)+1, 2**(w+1)-2]
  // gives:   sum = (a+b < 0) ? (a+b+2**w) : (a+b);
  Z3ASTHandleLIA maxUnsignedInt =
      liaUnsignedConst(llvm::APInt::getHighBitsSet(expr.getWidth() + 1, 1));
  llvm::APInt nullValue = llvm::APInt::getNullValue(expr.getWidth());

  Z3ASTHandleLIA condition = {Z3_mk_lt(ctx, expr, liaUnsignedConst(nullValue)),
                              ctx, 1, false};

  Z3_ast addArgs[] = {expr, maxUnsignedInt};
  Z3ASTHandleLIA addHandleLIA = {Z3_mk_add(ctx, 2, addArgs), ctx,
                                 expr.getWidth(), false};
  return {Z3_mk_ite(ctx, condition, addHandleLIA, expr), ctx, expr.getWidth(),
          false};
}

Z3ASTHandleLIA Z3LIABuilder::handleSignedOverflow(const Z3ASTHandleLIA &expr) {
  assert(expr.sign());

  // Signed overflow:
  // assumes: expr \in [-(2**w), 2**w-2]
  // gives:   sum = (a+b > 2**(w-1)-1) ? ((a+b)-2**w) : (a+b)
  Z3ASTHandleLIA maxSignedInt =
      liaSignedConst(llvm::APInt::getSignedMaxValue(expr.getWidth()));
  Z3ASTHandleLIA condition = {Z3_mk_gt(ctx, expr, maxSignedInt), ctx, 1, false};

  Z3_ast overflowASTArgs[] = {
      expr,
      liaUnsignedConst(llvm::APInt::getHighBitsSet(expr.getWidth() + 1, 1))};
  Z3ASTHandleLIA preparedExpr = {Z3_mk_sub(ctx, 2, overflowASTArgs), ctx,
                                 expr.getWidth(), true};

  return {Z3_mk_ite(ctx, condition, preparedExpr, expr), ctx, expr.getWidth(),
          true};
}

Z3ASTHandleLIA Z3LIABuilder::handleSignedUnderflow(const Z3ASTHandleLIA &expr) {
  assert(expr.sign());

  // Signed underflow:
  // assumes: expr \in [-(2**w), 2**w-2]
  // gives:   sum = (a+b < -2**(w-1) ? (a+b)+2**w : a+b)
  Z3ASTHandleLIA minSignedInt =
      liaSignedConst(llvm::APInt::getSignedMinValue(expr.getWidth()));
  Z3ASTHandleLIA condition = {Z3_mk_lt(ctx, expr, minSignedInt), ctx, 1, false};

  Z3_ast underflowASTArgs[] = {
      expr,
      liaUnsignedConst(llvm::APInt::getHighBitsSet(expr.getWidth() + 1, 1))};

  Z3ASTHandleLIA preparedExpr = {Z3_mk_add(ctx, 2, underflowASTArgs), ctx,
                                 expr.getWidth(), true};

  return {Z3_mk_ite(ctx, condition, preparedExpr, expr), ctx, expr.getWidth(),
          true};
}

Z3ASTHandleLIA Z3LIABuilder::liaGetTrue() {
  return {Z3_mk_true(ctx), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaGetFalse() {
  return {Z3_mk_false(ctx), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::castToSigned(const Z3ASTHandleLIA &expr) {
  if (expr.isBoolean()) {
    return liaIteExpr(expr, liaSignedConst(llvm::APInt(1, 1)),
                      liaSignedConst(llvm::APInt(1, 0)));
  }

  if (expr.sign()) {
    return expr;
  }

  Z3ASTHandleLIA signedExpr = {expr, ctx, expr.getWidth(), true};
  return handleSignedOverflow(signedExpr);
}

Z3ASTHandleLIA Z3LIABuilder::castToUnsigned(const Z3ASTHandleLIA &expr) {
  if (expr.isBoolean()) {
    return liaIteExpr(expr, liaUnsignedConst(llvm::APInt(1, 1)),
                      liaUnsignedConst(llvm::APInt(1, 0)));
  }

  if (!expr.sign()) {
    return expr;
  }

  Z3ASTHandleLIA unsignedExpr = {expr, ctx, expr.getWidth(), false};
  return handleUnsignedUnderflow(unsignedExpr);
}

Z3ASTHandleLIA Z3LIABuilder::castToBool(const Z3ASTHandleLIA &expr) {
  if (expr.isBoolean()) {
    return expr;
  }

  assert(expr.getWidth() == 1);
  return liaIteExpr(liaEq(expr, liaUnsignedConst(llvm::APInt(1, 0))),
                    liaGetFalse(), liaGetTrue());
}

Z3ASTHandleLIA Z3LIABuilder::liaUnsignedConst(const llvm::APInt &value) {
  std::string valueString = value.toString(10, false);
  Z3_string s = valueString.c_str();
  return {Z3_mk_numeral(ctx, s, liaSort()), ctx, value.getBitWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSignedConst(const llvm::APInt &value) {
  std::string valueString = value.toString(10, true);
  Z3_string s = valueString.c_str();
  return {Z3_mk_numeral(ctx, s, liaSort()), ctx, value.getBitWidth(), true};
}

Z3ASTHandleLIA Z3LIABuilder::liaUleExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA unsignedLhs = castToUnsigned(lhs);
  Z3ASTHandleLIA unsignedRhs = castToUnsigned(rhs);
  return {Z3_mk_le(ctx, unsignedLhs, unsignedRhs), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaUltExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA unsignedLhs = castToUnsigned(lhs);
  Z3ASTHandleLIA unsignedRhs = castToUnsigned(rhs);
  return {Z3_mk_lt(ctx, unsignedLhs, unsignedRhs), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSleExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_le(ctx, signedLhs, signedRhs), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSltExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_lt(ctx, signedLhs, signedRhs), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaAddExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  if (!lhs.sign() || !rhs.sign()) {
    // signed + unsigned
    // unsigned + unsigned
    const Z3_ast args[] = {castToUnsigned(lhs), castToUnsigned(rhs)};
    Z3ASTHandleLIA sumExpr(Z3_mk_add(ctx, 2, args), ctx, lhs.getWidth(), false);

    return handleUnsignedOverflow(sumExpr);
  } else {
    // signed + signed
    // overflow or underflow?
    // is this way better or make a cast to unsigned?
    const Z3_ast args[] = {lhs, rhs};
    Z3ASTHandleLIA sumExpr(Z3_mk_add(ctx, 2, args), ctx, lhs.getWidth(), true);
    return handleSignedUnderflow(handleSignedOverflow(sumExpr));
  }
}

Z3ASTHandleLIA Z3LIABuilder::liaSubExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  if (!lhs.sign() || !rhs.sign()) {
    // signed + unsigned
    // unsigned + unsigned
    const Z3_ast args[] = {castToUnsigned(lhs), castToUnsigned(rhs)};
    Z3ASTHandleLIA sumExpr(Z3_mk_sub(ctx, 2, args), ctx, lhs.getWidth(),
                           lhs.sign());

    return handleUnsignedUnderflow(sumExpr);
  } else {
    // signed + signed
    // overflow or underflow?
    // is this way better or make a cast to unsigned?
    const Z3_ast args[] = {castToSigned(lhs), castToSigned(rhs)};
    Z3ASTHandleLIA sumExpr(Z3_mk_sub(ctx, 2, args), ctx, lhs.getWidth(),
                           lhs.sign());
    return handleSignedUnderflow(handleSignedOverflow(sumExpr));
  }
}

Z3ASTHandleLIA Z3LIABuilder::liaMulExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  const Z3_ast args[] = {lhs, rhs};
  return {Z3_mk_mul(ctx, 2, args), ctx, lhs.getWidth(), lhs.sign()};
}

Z3ASTHandleLIA Z3LIABuilder::liaUdivExpr(const Z3ASTHandleLIA &lhs,
                                         const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA unsignedLhs = castToUnsigned(lhs);
  Z3ASTHandleLIA unsignedRhs = castToUnsigned(rhs);
  return {Z3_mk_div(ctx, unsignedLhs, unsignedRhs), ctx, lhs.getWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSdivExpr(const Z3ASTHandleLIA &lhs,
                                         const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA signedLhs = castToSigned(lhs);
  Z3ASTHandleLIA signedRhs = castToSigned(rhs);
  return {Z3_mk_div(ctx, signedLhs, signedRhs), ctx, lhs.getWidth(), true};
}

Z3ASTHandleLIA Z3LIABuilder::liaZextExpr(const Z3ASTHandleLIA &expr,
                                         unsigned width) {
  return {castToUnsigned(expr), ctx, width, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaSextExpr(const Z3ASTHandleLIA &expr,
                                         unsigned width) {
  return {castToSigned(expr), ctx, width, true};
}

Z3ASTHandleLIA Z3LIABuilder::liaAndExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  assert(lhs.getWidth() == rhs.getWidth() && lhs.getWidth() == 1);
  Z3_ast args[] = {castToBool(lhs), castToBool(rhs)};
  return {Z3_mk_and(ctx, 2, args), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaOrExpr(const Z3ASTHandleLIA &lhs,
                                       const Z3ASTHandleLIA &rhs) {
  assert(lhs.getWidth() == rhs.getWidth() && lhs.getWidth() == 1);
  Z3_ast args[] = {castToBool(lhs), castToBool(rhs)};
  return {Z3_mk_or(ctx, 2, args), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaXorExpr(const Z3ASTHandleLIA &lhs,
                                        const Z3ASTHandleLIA &rhs) {
  assert(lhs.getWidth() == rhs.getWidth() && lhs.getWidth() == 1);
  return {Z3_mk_xor(ctx, castToBool(lhs), castToBool(rhs)), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaNotExpr(const Z3ASTHandleLIA &expr) {
  assert(expr.getWidth() == 1);
  return {Z3_mk_not(ctx, castToBool(expr)), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaEq(const Z3ASTHandleLIA &lhs,
                                   const Z3ASTHandleLIA &rhs) {
  return {Z3_mk_eq(ctx, lhs, rhs), ctx, 1, false};
}

Z3ASTHandleLIA Z3LIABuilder::liaIteExpr(const Z3ASTHandleLIA &condition,
                                        const Z3ASTHandleLIA &whenTrue,
                                        const Z3ASTHandleLIA &whenFalse) {

  if (whenTrue.sign() != whenFalse.sign()) {
    return {Z3_mk_ite(ctx, castToBool(condition), castToUnsigned(whenTrue),
                      castToUnsigned(whenFalse)),
            ctx, whenTrue.getWidth(), false};
  }
  return {Z3_mk_ite(ctx, castToBool(condition), whenTrue, whenFalse), ctx,
          whenTrue.getWidth(), whenTrue.sign()};
}

Z3ASTHandleLIA Z3LIABuilder::liaConcatExpr(const Z3ASTHandleLIA &lhs,
                                           const Z3ASTHandleLIA &rhs) {
  Z3ASTHandleLIA shift =
      liaUnsignedConst(llvm::APInt::getHighBitsSet(rhs.getWidth() + 1, 1));
  Z3_ast args[] = {castToUnsigned(lhs), shift};

  Z3ASTHandleLIA shiftedLhs = {Z3_mk_mul(ctx, 2, args), ctx,
                               lhs.getWidth() + rhs.getWidth(), false};
  Z3_ast sumArgs[] = {shiftedLhs, castToUnsigned(rhs)};

  return {Z3_mk_add(ctx, 2, sumArgs), ctx, shiftedLhs.getWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::liaGetInitialArray(const Array *root) {
  assert(root);
  Z3ASTHandleLIA array_expr;
  bool hashed = arrHashLIA.lookupArrayExpr(root, array_expr);

  if (!hashed) {
    // Unique arrays by name, so we make sure the name is unique by
    // using the size of the array hash as a counter.
    std::string unique_id = llvm::utostr(arrHashLIA._array_hash.size());
    std::string unique_name = root->name + unique_id;
    if (ref<ConstantWithSymbolicSizeSource> constantWithSymbolicSizeSource =
            dyn_cast<ConstantWithSymbolicSizeSource>(root->source)) {
      array_expr = liaBuildConstantArray(
          unique_name.c_str(),
          llvm::APInt(root->getRange(),
                      constantWithSymbolicSizeSource->defaultValue));
    } else {
      array_expr = liaBuildArray(unique_name.c_str(), root->getRange());
    }

    if (root->isConstantArray() && constant_array_assertions.count(root) == 0) {
      std::vector<Z3ASTHandle> array_assertions;
      for (unsigned i = 0, e = root->constantValues.size(); i != e; ++i) {
        // construct(= (select i root) root->value[i]) to be asserted in
        // Z3Solver.cpp
        Z3ASTHandleLIA array_value = constructLIA(root->constantValues[i]);
        array_assertions.push_back(liaEq(
            liaReadExpr(array_expr,
                        liaUnsignedConst(llvm::APInt(root->getDomain(), i))),
            array_value));
      }
      constant_array_assertions[root] = std::move(array_assertions);
    }

    arrHashLIA.hashArrayExpr(root, array_expr);
  }

  return array_expr;
}

Z3ASTHandle Z3LIABuilder::getInitialRead(const Array *root, unsigned index) {
  return liaReadExpr(liaGetInitialArray(root),
                     liaUnsignedConst(llvm::APInt(root->getDomain(), index)));
}

Z3ASTHandleLIA Z3LIABuilder::liaGetArrayForUpdate(const Array *root,
                                                  const UpdateNode *un) {
  if (!un) {
    return liaGetInitialArray(root);
  } else {
    // FIXME: This really needs to be non-recursive.
    Z3ASTHandleLIA un_expr;
    bool hashed = arrHashLIA.lookupUpdateNodeExpr(un, un_expr);

    if (!hashed) {
      un_expr = liaWriteExpr(liaGetArrayForUpdate(root, un->next.get()),
                             constructLIA(un->index), constructLIA(un->value));

      arrHashLIA.hashUpdateNodeExpr(un, un_expr);
    }

    return un_expr;
  }
}

Z3ASTHandleLIA Z3LIABuilder::liaBuildArray(const char *name, unsigned width) {
  Z3SortHandle t = getArraySort(liaSort(), liaSort());
  Z3_symbol s = Z3_mk_string_symbol(ctx, const_cast<char *>(name));
  return {Z3_mk_const(ctx, s, t), ctx, width, false};
}

Z3ASTHandleLIA
Z3LIABuilder::liaBuildConstantArray(const char *name,
                                    const llvm::APInt &defaultValue) {
  Z3ASTHandleLIA liaDefaultValue = liaUnsignedConst(defaultValue);
  return {Z3_mk_const_array(ctx, liaSort(), liaDefaultValue), ctx,
          liaDefaultValue.getWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::liaWriteExpr(const Z3ASTHandleLIA &array,
                                          const Z3ASTHandleLIA &index,
                                          const Z3ASTHandleLIA &value) {
  return {Z3_mk_store(ctx, array, index, castToUnsigned(value)), ctx,
          array.getWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::liaReadExpr(const Z3ASTHandleLIA &array,
                                         const Z3ASTHandleLIA &index) {
  return {Z3_mk_select(ctx, array, index), ctx, array.getWidth(), false};
}

Z3ASTHandleLIA Z3LIABuilder::constructLIA(const ref<Expr> &e) {
  if (!Z3HashConfig::UseConstructHashZ3 || isa<ConstantExpr>(e)) {
    return constructActualLIA(e);
  } else {
    auto it = constructedLIA.find(e);
    if (it != constructedLIA.end()) {
      return it->second;
    } else {
      Z3ASTHandleLIA res = constructActualLIA(e);
      if (!isBroken) {
        constructedLIA.insert(std::make_pair(e, res));
      }
      return res;
    }
  }
}

bool Z3LIABuilder::isContigousConstantRead(const ref<ConcatExpr> &ce) {
  ref<Expr> lastRightSon = nullptr;
  uint64_t lastIdx = 0;
  const Array *concatArray = nullptr;

  for (ref<ConcatExpr> ceIt = ce; ceIt;
       ceIt = dyn_cast<ConcatExpr>(ceIt->getRight())) {
    const Array *underlyingArray = nullptr;
    uint64_t idx = 0;

    if (ref<ReadExpr> re = dyn_cast<ReadExpr>(ceIt->getLeft())) {
      underlyingArray = re->updates.root;
      if (ref<ConstantExpr> idxExpr = dyn_cast<ConstantExpr>(re->index)) {
        idx = idxExpr->getZExtValue();
      } else {
        return false;
      }
    } else {
      return false;
    }

    if (concatArray == nullptr) {
      underlyingArray = concatArray;
    } else if (concatArray == underlyingArray) {
      if (idx + 1 != lastIdx) {
        return false;
      }
    } else {
      return false;
    }
    lastIdx = idx;

    lastRightSon = ceIt->getRight();
  }
  return isa<ReadExpr>(lastRightSon);
}

static std::vector<const Array *> allArrays(const ref<ConcatExpr> &concat) {
  std::vector<ref<ReadExpr>> readsInConcat;
  findReads(concat, false, readsInConcat);
  std::vector<const Array *> result;
  for (ref<ReadExpr> read : readsInConcat) {
    result.push_back(read->updates.root);
  }
  return result;
}

static const Array *anyArray(const ref<ConcatExpr> &concat) {
  for (ref<ConcatExpr> ceIt = concat; ceIt;
       ceIt = dyn_cast<ConcatExpr>(ceIt->getRight())) {
    if (ref<ReadExpr> re = dyn_cast<ReadExpr>(ceIt->getLeft())) {
      return re->updates.root;
    }
  }
  return nullptr;
}

static ref<ConstantExpr>
firstIdxFromConstantRead(const ref<ConcatExpr> &concat) {
  std::vector<ref<ReadExpr>> readsInConcat;
  findReads(concat, false, readsInConcat);
  uint64_t result = -1;
  for (ref<ReadExpr> read : readsInConcat) {
    if (ref<ConstantExpr> ce = dyn_cast<ConstantExpr>(read->index)) {
      result = std::min(result, ce->getZExtValue());
    } else {
      return nullptr;
    }
  }
  return ConstantExpr::create(result, readsInConcat.front()->getWidth());
}

static ref<ConstantExpr> readsIn(const ref<ConcatExpr> &concat) {
  if (ref<ReadExpr> re = dyn_cast<ReadExpr>(concat->getLeft())) {
    if (concat->getWidth() % re->getWidth() != 0) {
      return nullptr;
    }
    uint64_t numWidth = concat->getWidth() / re->getWidth();
    return ConstantExpr::create(numWidth, concat->getWidth());
  }
  return nullptr;
}

void Z3LIABuilder::loadReads(const std::vector<ref<ConcatExpr>> &concats) {
  readExprs.clear();
  // which offsets should not be optimized
  std::unordered_set<const Array *> toDelete;
  std::unordered_map<const Array *, std::vector<uint64_t>> partToDelete;

  for (const ref<ConcatExpr> &singleConcat : concats) {
    const Array *array = anyArray(singleConcat);
    /// either read from constant array or a corrupted concat
    if (array == nullptr) {
      assert(false && "this should not happen");
      continue;
    }

    // If not, we need to find which indexes were used to read from
    // to forbid oprimizations for that arrays.
    if (!isContigousConstantRead(singleConcat)) {
      for (const Array *array : allArrays(singleConcat)) {
        toDelete.insert(array);
      }
      continue;
    }

    ref<ConstantExpr> firstIdx = firstIdxFromConstantRead(singleConcat);
    ref<ConstantExpr> concatSize = readsIn(singleConcat);
    assert(firstIdx && concatSize);

    uint64_t firstIdxConst = firstIdx->getZExtValue();
    uint64_t concatSizeConst = concatSize->getZExtValue();

    if (!readExprs[array].emplace(firstIdxConst, concatSizeConst).second) {
      partToDelete[array].push_back(firstIdxConst);
    }
  }

  for (const Array *array : toDelete) {
    readExprs.erase(array);
  }

  for (const auto &it : partToDelete) {
    if (readExprs.count(it.first)) {
      for (auto &itt : it.second) {
        readExprs.at(it.first).erase(itt);
      }
    }
  }
}

bool Z3LIABuilder::isNonOverlapping(const ref<ConcatExpr> &concat) {
  if (!isContigousConstantRead(concat)) {
    return false;
  }

  const Array *array = anyArray(concat);
  ref<ConstantExpr> firstIdx = firstIdxFromConstantRead(concat);
  ref<ConstantExpr> concatSize = readsIn(concat);
  assert(firstIdx && concatSize);

  uint64_t firstIdxConst = firstIdx->getZExtValue();
  uint64_t concatSizeConst = concatSize->getZExtValue();

  if (!readExprs.count(array)) {
    return false;
  }

  const std::map<uint64_t, uint64_t> &atArray = readExprs.at(array);
  auto itAtArray = atArray.lower_bound(firstIdxConst);
  if (itAtArray == atArray.end()) {
    // should find that array
    return false;
  }

  if (itAtArray->first == firstIdxConst &&
      itAtArray->second == concatSizeConst) {
    // iterate forward
    auto nextItAtArray = std::next(itAtArray);
    if (nextItAtArray != atArray.end() &&
        firstIdxConst + concatSizeConst + 1 >= std::next(itAtArray)->first) {
      return false;
    }
  } else {
    return false;
  }

  // iterate backward to find all intersections
  if (itAtArray != atArray.begin()) {
    --itAtArray;
    if (itAtArray->first + itAtArray->second + 1 >= firstIdxConst) {
      return false;
    }
  }

  return true;
}

Z3ASTHandle Z3LIABuilder::construct(ref<Expr> e, int *width_out) {
  isBroken = false;
  Z3ASTHandleLIA result = constructLIA(e);
  if (width_out) {
    *width_out = result.getWidth();
  }
  if (e->getWidth() == 1) {
    return castToBool(result);
  }

  return result;
}

/** if *width_out!=1 then result is a bitvector,
    otherwise it is a bool */
Z3ASTHandleLIA Z3LIABuilder::constructActualLIA(const ref<Expr> &e) {
  ++stats::queryConstructs;

  switch (e->getKind()) {
  case Expr::Constant: {
    ref<ConstantExpr> CE = cast<ConstantExpr>(e);
    // Coerce to bool if necessary.
    return liaUnsignedConst(CE->getAPValue());
  }

  // Special
  case Expr::NotOptimized: {
    ref<NotOptimizedExpr> noe = cast<NotOptimizedExpr>(e);
    return constructLIA(noe->src);
  }

  case Expr::Read: {
    ref<ReadExpr> re = cast<ReadExpr>(e);
    assert(re && re->updates.root);
    // value received as read by index must be restricted

    auto readExpr = liaReadExpr(
        liaGetArrayForUpdate(re->updates.root, re->updates.head.get()),
        constructLIA(re->index));

    sideConstraints.push_back(
        liaUleExpr(liaUnsignedConst(llvm::APInt(CHAR_BIT, 0)), readExpr));
    sideConstraints.push_back(liaUleExpr(
        readExpr, liaUnsignedConst(llvm::APInt::getMaxValue(CHAR_BIT))));

    return readExpr;
  }

  case Expr::Select: {
    ref<SelectExpr> se = cast<SelectExpr>(e);
    Z3ASTHandleLIA cond = constructLIA(se->cond);
    Z3ASTHandleLIA tExpr = constructLIA(se->trueExpr);
    Z3ASTHandleLIA fExpr = constructLIA(se->falseExpr);
    return liaIteExpr(cond, tExpr, fExpr);
  }

  case Expr::Concat: {
    ref<ConcatExpr> ce = cast<ConcatExpr>(e);
    int numKids = static_cast<int>(ce->getNumKids());

    if (isNonOverlapping(ce) && readsIn(ce)->getZExtValue() <= sizeof(uint64_t)) {
      const Array *concatArray = anyArray(ce);
      
      uint64_t firstReadIdx = firstIdxFromConstantRead(ce)->getZExtValue();

      std::map<uint64_t, Z3ASTHandleLIA> &atArray = optimizedReads[concatArray];
      if (atArray.count(firstReadIdx) == 0) {
        std::string unique_name = concatArray->getName() + "#" + std::to_string(atArray.size());

        Z3_symbol symbol = Z3_mk_string_symbol(ctx, unique_name.c_str());
        Z3ASTHandleLIA arrayInteger(Z3_mk_const(ctx, symbol, liaSort()), ctx,
                                      ce->getWidth(), false);

        sideConstraints.push_back(liaUleExpr(
            liaUnsignedConst(llvm::APInt(ce->getWidth(), 0)), arrayInteger));
        sideConstraints.push_back(liaUleExpr(
            arrayInteger,
            liaUnsignedConst(llvm::APInt::getMaxValue(ce->getWidth()))));
        atArray[firstReadIdx] = arrayInteger;
      }

      return atArray[firstReadIdx];
    }

    Z3ASTHandleLIA res = constructLIA(ce->getKid(0));

    for (int i = 1; i < numKids; ++i) {
      Z3ASTHandleLIA kidExpr = constructLIA(ce->getKid(i));
      res = liaConcatExpr(res, kidExpr);
    }
    return res;
  }

    // Casting

  case Expr::ZExt: {
    ref<CastExpr> ce = cast<CastExpr>(e);
    Z3ASTHandleLIA src = constructLIA(ce->src);
    if (ce->getWidth() == 1) {
      return liaIteExpr(src, liaUnsignedConst(llvm::APInt(1, 1)),
                        liaUnsignedConst(llvm::APInt(1, 0)));
    } else {
      return liaZextExpr(src, ce->getWidth());
    }
  }

  case Expr::SExt: {
    ref<CastExpr> ce = cast<CastExpr>(e);
    Z3ASTHandleLIA src = constructLIA(ce->src);
    if (ce->getWidth() == 1) {
      return liaIteExpr(src, liaSignedConst(llvm::APInt(1, -1)),
                        liaSignedConst(llvm::APInt(1, 0)));
    } else {
      return liaSextExpr(src, ce->getWidth());
    }
  }

  // Arithmetic
  case Expr::Add: {
    ref<AddExpr> ae = cast<AddExpr>(e);
    Z3ASTHandleLIA left = constructLIA(ae->left);
    Z3ASTHandleLIA right = constructLIA(ae->right);
    return liaAddExpr(left, right);
  }

  case Expr::Sub: {
    ref<SubExpr> se = cast<SubExpr>(e);
    Z3ASTHandleLIA left = constructLIA(se->left);
    Z3ASTHandleLIA right = constructLIA(se->right);
    return liaSubExpr(left, right);
  }

  // Bitwise
  case Expr::Not: {
    ref<NotExpr> ne = cast<NotExpr>(e);
    Z3ASTHandleLIA expr = constructLIA(ne->expr);
    if (expr.getWidth() == 1) {
      return liaNotExpr(expr);
    } else {
      isBroken = true;
      return liaUnsignedConst(llvm::APInt(e->getWidth(), 0));
    }
  }

  case Expr::And: {
    ref<AndExpr> ae = cast<AndExpr>(e);
    Z3ASTHandleLIA left = constructLIA(ae->left);
    Z3ASTHandleLIA right = constructLIA(ae->right);
    if (left.getWidth() == 1) {
      return liaAndExpr(left, right);
    } else {
      isBroken = true;
      return liaUnsignedConst(llvm::APInt(e->getWidth(), 0));
    }
  }

  case Expr::Or: {
    ref<OrExpr> oe = cast<OrExpr>(e);
    Z3ASTHandleLIA left = constructLIA(oe->left);
    Z3ASTHandleLIA right = constructLIA(oe->right);
    if (left.getWidth() == 1) {
      return liaOrExpr(left, right);
    } else {
      isBroken = true;
      return liaUnsignedConst(llvm::APInt(e->getWidth(), 0));
    }
  }

  case Expr::Xor: {
    ref<XorExpr> xe = cast<XorExpr>(e);
    Z3ASTHandleLIA left = constructLIA(xe->left);
    Z3ASTHandleLIA right = constructLIA(xe->right);

    if (left.getWidth() == 1) {
      // XXX check for most efficient?
      return liaXorExpr(left, right);
    } else {
      isBroken = true;
      return liaUnsignedConst(llvm::APInt(e->getWidth(), 0));
    }
  }
    // Comparison

  case Expr::Eq: {
    ref<EqExpr> ee = cast<EqExpr>(e);
    Z3ASTHandleLIA left = constructLIA(ee->left);
    Z3ASTHandleLIA right = constructLIA(ee->right);
    if (left.getWidth() == 1) {
      if (ref<ConstantExpr> CE = dyn_cast<ConstantExpr>(ee->left)) {
        if (CE->isTrue()) {
          return castToBool(right);
        }
        return liaNotExpr(right);
      } else {
        return liaEq(castToBool(left), castToBool(right));
      }
    } else {
      return liaEq(left, right);
    }
  }

  case Expr::Ult: {
    ref<UltExpr> ue = cast<UltExpr>(e);
    Z3ASTHandleLIA left = constructLIA(ue->left);
    Z3ASTHandleLIA right = constructLIA(ue->right);
    return liaUltExpr(left, right);
  }

  case Expr::Ule: {
    ref<UleExpr> ue = cast<UleExpr>(e);
    Z3ASTHandleLIA left = constructLIA(ue->left);
    Z3ASTHandleLIA right = constructLIA(ue->right);
    return liaUleExpr(left, right);
  }

  case Expr::Slt: {
    ref<SltExpr> se = cast<SltExpr>(e);
    Z3ASTHandleLIA left = constructLIA(se->left);
    Z3ASTHandleLIA right = constructLIA(se->right);
    return liaSltExpr(left, right);
  }

  case Expr::Sle: {
    ref<SleExpr> se = cast<SleExpr>(e);
    Z3ASTHandleLIA left = constructLIA(se->left);
    Z3ASTHandleLIA right = constructLIA(se->right);
    return liaSleExpr(left, right);
  }

// case Expr::Mul:
// case Expr::UDiv:
// case Expr::SDiv:
// case Expr::URem:
// case Expr::SRem:
// case Expr::Shl:
// case Expr::LShr:
// case Expr::Extract:
// case Expr::AShr:

// unused due to canonicalization
#if 0
  case Expr::Ne:
  case Expr::Ugt:
  case Expr::Uge:
  case Expr::Sgt:
  case Expr::Sge:
#endif

  default:
    isBroken = true;
    return liaUnsignedConst(llvm::APInt(e->getWidth(), 0));
    // assert(0 && "unhandled Expr type");
  }
}
