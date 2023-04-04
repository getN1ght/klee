//===-- FastCexSolver.cpp -------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cex-solver"
#include "klee/Solver/Solver.h"

#include "klee/ADT/SparseStorage.h"
#include "klee/Expr/Constraints.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprEvaluator.h"
#include "klee/Expr/ExprRangeEvaluator.h"
#include "klee/Expr/ExprVisitor.h"
#include "klee/Solver/IncompleteSolver.h"
#include "klee/Support/Debug.h"
#include "klee/Support/ErrorHandling.h"

#include "llvm/ADT/APInt.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <map>
#include <set>
#include <sstream>
#include <vector>

using namespace klee;

// Hacker's Delight, pgs 58-63
static llvm::APInt minOR(llvm::APInt a, llvm::APInt b, llvm::APInt c,
                         llvm::APInt d) {
  assert(a.getBitWidth() == c.getBitWidth());
  if (a == b) {
    return llvm::APIntOps::umin(a | c, a | d);
  }
  if (c == d) {
    return llvm::APIntOps::umin(a | c, b | c);
  }

  llvm::APInt m =
      llvm::APInt::getOneBitSet(a.getBitWidth(), a.getBitWidth() - 1);
  while (m.getBoolValue()) {
    if ((a.reverseBits() & c & m).getBoolValue()) {
      llvm::APInt temp = (a | m) & -m;
      if (temp.ule(b)) {
        a = temp;
        break;
      }
    } else if ((a & c.reverseBits() & m).getBoolValue()) {
      llvm::APInt temp = (c | m) & -m;
      if (temp.ule(d)) {
        c = temp;
        break;
      }
    }
    m = m.lshr(1);
  }

  return a | c;
}
static llvm::APInt maxOR(llvm::APInt a, llvm::APInt b, llvm::APInt c,
                         llvm::APInt d) {
  assert(a.getBitWidth() == c.getBitWidth());
  if (a == b) {
    return llvm::APIntOps::umax(a | c, a | d);
  }
  if (c == d) {
    return llvm::APIntOps::umax(a | c, b | c);
  }

  llvm::APInt m =
      llvm::APInt::getOneBitSet(a.getBitWidth(), a.getBitWidth() - 1);

  while (m.getBoolValue()) {
    if ((b & d & m).getBoolValue()) {
      llvm::APInt temp = (b - m) | (m - 1);
      if (temp.uge(a)) {
        b = temp;
        break;
      }
      temp = (d - m) | (m - 1);
      if (temp.uge(c)) {
        d = temp;
        break;
      }
    }
    m = m.lshr(1);
  }

  return b | d;
}

static llvm::APInt minAND(llvm::APInt a, llvm::APInt b, llvm::APInt c,
                          llvm::APInt d) {
  assert(a.getBitWidth() == c.getBitWidth());
  if (a == b) {
    return llvm::APIntOps::umin(a & c, a & d);
  }
  if (c == d) {
    return llvm::APIntOps::umin(a & c, b & c);
  }

  llvm::APInt m =
      llvm::APInt::getOneBitSet(a.getBitWidth(), a.getBitWidth() - 1);

  while (m.getBoolValue()) {
    if ((~a & ~c & m).getBoolValue()) {
      llvm::APInt temp = (a | m) & -m;
      if (temp.ule(b)) {
        a = temp;
        break;
      }
      temp = (c | m) & -m;
      if (temp.ule(d)) {
        c = temp;
        break;
      }
    }
    m = m.lshr(1);
  }

  return a & c;
}
static llvm::APInt maxAND(llvm::APInt a, llvm::APInt b, llvm::APInt c,
                          llvm::APInt d) {
  assert(a.getBitWidth() == c.getBitWidth());
  if (a == b) {
    return llvm::APIntOps::umax(a & c, a & d);
  }
  if (c == d) {
    return llvm::APIntOps::umax(a & c, b & c);
  }

  llvm::APInt m =
      llvm::APInt::getOneBitSet(a.getBitWidth(), a.getBitWidth() - 1);

  while (m.getBoolValue()) {
    if ((b & ~d & m).getBoolValue()) {
      llvm::APInt temp = (b & ~m) | (m - 1);
      if (temp.uge(a)) {
        b = temp;
        break;
      }
    } else if ((~b & d & m).getBoolValue()) {
      llvm::APInt temp = (d & ~m) | (m - 1);
      if (temp.uge(c)) {
        d = temp;
        break;
      }
    }
    m = m.lshr(1);
  }

  return b & d;
}

///

class ValueRange {
  friend class ExprRangeEvaluator<ValueRange>;

public:
  // Elementary range representing simple segment range, i.e.
  // all values from m_min to m_max inclusive.
  struct ValueSegment {
    // Maintain invariant that m_min <= m_max.
    llvm::APInt m_min, m_max;
    ValueSegment(llvm::APInt _m_min, llvm::APInt _m_max)
        : m_min(_m_min), m_max(_m_max) {}

    bool isFixed() const { return m_min == m_max; }

    // Lexicographical comparison.
    bool operator<(const ValueSegment &rhs) const {
      if (m_min == rhs.m_min) {
        return m_max.ult(rhs.m_max);
      }
      return m_min.ult(rhs.m_min);
    }
    bool operator==(const ValueSegment &rhs) const {
      return m_min == rhs.m_min && m_max == rhs.m_max;
    }
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                         const ValueSegment &seg) {
      return os << "[" << seg.m_min << "," << seg.m_max << "]";
    }
  };

  // FIXME:
public:
  // Represent set of **non-intersecting** segments (with both sides inclusion,
  // e.g. [1, 2]) of ranges for this value with common width. Segments with
  // neighbouting bounds should be concat in one (e.g. set{[0, 1], [2, 3]} is
  // forbidden, make set{[0, 3]} instead). Nested segments are also forbidden.
  std::set<ValueSegment> segments;

  // Width of segments. Note that empty ValueRange does not have
  // width, so reading from this variable may lead to unspecified
  // behavior without.
  unsigned width;

  // Adds segment to ValueRange and unite with previos if they differ on 1.
  // It can intersect previously added segments, so we will merge them in this
  // case. It performs every operation in O(log(n)) amorthized.
  void addSegment(ValueSegment seg) {
    typename std::set<ValueSegment>::iterator lowerSegment =
        segments.upper_bound(seg);

    // To handle such case make a step back.
    // ...[m_min, m_max]...
    //  [...]..[lower, upper]...
    if (lowerSegment != segments.begin()) {
      --lowerSegment;
    }

    // If current segment [s_min, s_max] satisfies the following conditions:
    //  * s_max >= m_min - 1
    //  * s_min <= m_max + 1
    // then add it into one great segment, taking care about overflows.
    std::vector<ValueSegment> eraseSegments;
    for (; lowerSegment != segments.end() &&
           lowerSegment->m_min.ule(seg.m_max.isMaxValue() ? seg.m_max
                                                          : seg.m_max + 1);
         ++lowerSegment) {
      if (lowerSegment->m_max.uge(seg.m_min.getBoolValue() ? seg.m_min - 1
                                                           : seg.m_min)) {
        eraseSegments.push_back(*lowerSegment);
      }
    }

    if (!eraseSegments.empty()) {
      seg.m_min = llvm::APIntOps::umin(seg.m_min, eraseSegments.front().m_min);
      seg.m_max = llvm::APIntOps::umax(seg.m_max, eraseSegments.back().m_max);
    }

    for (const ValueSegment &seg : eraseSegments) {
      segments.erase(seg);
    }
    segments.insert(seg);
  }

public:
  ValueRange() noexcept {}
  ValueRange(const llvm::APInt &_min, const llvm::APInt &_max) noexcept
      : width(_min.getBitWidth()) {
    assert(_min.getBitWidth() == _max.getBitWidth());
    addSegment(ValueSegment(_min, _max));
  }
  explicit ValueRange(const llvm::APInt &value) noexcept
      : ValueRange(value, value) {}
  ValueRange(const ref<ConstantExpr> &ce) : ValueRange(ce->getAPValue()) {}

  ValueRange(const ValueRange &other) noexcept = default;
  ValueRange &operator=(const ValueRange &other) noexcept = default;
  ValueRange(ValueRange &&other) noexcept = default;
  ValueRange &operator=(ValueRange &&other) noexcept = default;

  void print(llvm::raw_ostream &os) const {
    if (isFixed()) {
      os << segments.begin()->m_min;
    } else {
      os << "[ ";
      bool firstIteration = true;
      for (const ValueSegment &seg : segments) {
        if (!firstIteration) {
          os << ", ";
        }
        os << seg;
        firstIteration = false;
      }
      os << " ]";
    }
  }

  unsigned bitWidth() const { return width; }

  bool isEmpty() const noexcept { return segments.empty(); }
  bool contains(const llvm::APInt &value) const {
    return this->intersects(ValueRange(value));
  }
  bool intersects(const ValueRange &b) const {
    return !this->set_intersection(b).isEmpty();
  }

  bool isFullRange() const noexcept {
    if (segments.size() != 1) {
      return false;
    }
    return segments.begin()->m_min == 0 &&
           segments.begin()->m_max == llvm::APInt::getAllOnesValue(bitWidth());
  }

  ValueRange set_intersection(const ValueRange &b) const {
    assert(bitWidth() == b.bitWidth());

    ValueRange intersectedRange;
    intersectedRange.width = bitWidth();

    // Use 2 pointers algorithm to achieve O(n) solution.
    for (std::set<ValueSegment>::const_iterator itLHS = segments.cbegin(),
                                                itEndLHS = segments.cend(),
                                                itRHS = b.segments.cbegin(),
                                                itEndRHS = b.segments.cend();
         itLHS != itEndLHS; ++itLHS) {
      // Move iterator from rhs ValueRange forward until meet intersection.
      while (itRHS != itEndRHS && itRHS->m_max.ult(itLHS->m_min)) {
        ++itRHS;
      }

      // Move forward while intersection exists.
      while (itRHS != itEndRHS && itRHS->m_min.ule(itLHS->m_max)) {
        intersectedRange.addSegment(
            ValueSegment(llvm::APIntOps::umax(itLHS->m_min, itRHS->m_min),
                         llvm::APIntOps::umin(itLHS->m_max, itRHS->m_max)));
        ++itRHS;
      }

      // We could overstep segment. In the next iteration
      // check previous one.
      if (itRHS != b.segments.cbegin()) {
        --itRHS;
      }
    }

    return intersectedRange;
  }

  ValueRange set_union(const ValueRange &b) const {
    assert(bitWidth() == b.bitWidth());

    ValueRange unitedRange = *this;
    for (const ValueSegment &seg : b.segments) {
      unitedRange.addSegment(seg);
    }
    return unitedRange;
  }

  ValueRange set_complement() const {
    ValueRange complement;
    complement.width = bitWidth();

    llvm::APInt segmentBegin = llvm::APInt(bitWidth(), 0);
    for (const ValueSegment &seg : segments) {
      if (segmentBegin != seg.m_min) {
        complement.addSegment(ValueSegment(segmentBegin, seg.m_min - 1));
      }
      segmentBegin = seg.m_max + 1;
    }

    // If overflow happened, then last segment in ValueRange is maximum
    // value for such width, so the last segment does not exist.
    if (segmentBegin != 0) {
      complement.addSegment(
          ValueSegment(segmentBegin, llvm::APInt::getMaxValue(bitWidth())));
    }
    return complement;
  }

  ValueRange set_difference(const ValueRange &b) const {
    // This result can be achieved without unneccessary complement
    // operation, but that is much easier and have the same time
    // complexity O(n).
    return set_intersection(b.set_complement());
  }

  ValueRange binaryAnd(const ValueRange &b) const {
    assert(!isEmpty() && !b.isEmpty() && "XXX");
    ValueRange andedRange;
    andedRange.width = bitWidth();

    for (const ValueSegment &segLHS : segments) {
      for (const ValueSegment &segRHS : b.segments) {
        llvm::APInt lBound =
            minAND(segLHS.m_min, segLHS.m_max, segRHS.m_min, segRHS.m_max);
        llvm::APInt rBound =
            maxAND(segLHS.m_min, segLHS.m_max, segRHS.m_min, segRHS.m_max);
        if (lBound.ule(rBound)) {
          andedRange.addSegment(ValueSegment(lBound, rBound));
        }
      }
    }
    return andedRange;
  }

  ValueRange binaryAnd(const llvm::APInt &b) const {
    return binaryAnd(ValueRange(b));
  }

  ValueRange binaryOr(ValueRange b) const {
    assert(!isEmpty() && !b.isEmpty() && "XXX");
    ValueRange oredRange;
    oredRange.width = bitWidth();

    for (const ValueSegment &segLHS : segments) {
      for (const ValueSegment &segRHS : b.segments) {
        llvm::APInt lBound =
            minOR(segLHS.m_min, segLHS.m_max, segRHS.m_min, segRHS.m_max);
        llvm::APInt rBound =
            maxOR(segLHS.m_min, segLHS.m_max, segRHS.m_min, segRHS.m_max);
        if (lBound.ule(rBound)) {
          oredRange.addSegment(ValueSegment(lBound, rBound));
        }
      }
    }
    return oredRange;
  }

  ValueRange binaryOr(const llvm::APInt &b) const {
    return binaryOr(ValueRange(b));
  }

  ValueRange binaryXor(ValueRange b) const {
    assert(bitWidth() == b.bitWidth());
    // TODO: we can make a more precise approximation here
    // using one more algorithm from the book mentioned above.

    ValueRange xoredRange;
    xoredRange.width = bitWidth();
    for (const ValueSegment &segA : segments) {
      for (const ValueSegment &segB : b.segments) {
        llvm::APInt t = segA.m_max | segB.m_max;
        if (!t.isPowerOf2()) {
          t = llvm::APInt::getOneBitSet(
              t.getBitWidth(), t.getBitWidth() - t.countLeadingZeros());
        }
        xoredRange.addSegment(ValueSegment(
            llvm::APInt::getNullValue(t.getBitWidth()), (t << 1) - 1));
      }
    }
    return xoredRange;
  }

  ValueRange binaryShiftLeft(unsigned bits) const {
    ValueRange shiftedRange;
    shiftedRange.width = bitWidth();
    for (const ValueSegment &seg : segments) {
      shiftedRange.addSegment(
          ValueSegment(seg.m_min.shl(bits), seg.m_max.shl(bits)));
    }
    return shiftedRange;
  }

  ValueRange binaryShiftRight(unsigned bits) const {
    ValueRange shiftedRange;
    shiftedRange.width = bitWidth();
    for (const ValueSegment &seg : segments) {
      ValueSegment newShiftedSeg(seg.m_min.lshr(bits), seg.m_max.lshr(bits));
      shiftedRange.addSegment(
          ValueSegment(seg.m_min.lshr(bits), seg.m_max.lshr(bits)));
    }
    return shiftedRange;
  }

  ValueRange concat(ValueRange b, unsigned bits) const {
    ValueRange concattedRange = zextOrTrunc(bitWidth() + bits);
    concattedRange = concattedRange.binaryShiftLeft(bits);
    return concattedRange.binaryOr(b.zextOrTrunc(bitWidth() + bits));
  }
  ValueRange extract(std::uint64_t lowBit, std::uint64_t maxBit) const {
    assert(!isEmpty());
    ValueRange extractRange =
        binaryShiftRight(width - maxBit)
            .binaryAnd(llvm::APInt::getLowBitsSet(width, maxBit - lowBit));
    extractRange = extractRange.zextOrTrunc(maxBit - lowBit);
    assert(!extractRange.isEmpty());
    return extractRange;
  }

  ValueRange add(const ValueRange &b) const {
    ValueRange addedRange;
    addedRange.width = bitWidth();
    for (const ValueSegment &segA : segments) {
      for (const ValueSegment &segB : b.segments) {
        ValueSegment segNew(segA.m_min + segB.m_min, segA.m_max + segB.m_max);

        // If maximum overflows and minimum not, then we will add ranges
        // [new_min, 111..111], [0, new_max].
        if (segNew.m_max.ult(segA.m_max) && segNew.m_min.uge(segA.m_min)) {
          addedRange.addSegment(ValueRange::ValueSegment(
              llvm::APInt(bitWidth(), 0), segNew.m_max));
          segNew.m_max = llvm::APInt::getAllOnesValue(bitWidth());
        }
        addedRange.addSegment(segNew);
      }
    }
    return addedRange;
  }

  ValueRange sub(const ValueRange &b) const {
    ValueRange subtractRange;
    subtractRange.width = bitWidth();
    for (const ValueSegment &seg : b.segments) {
      subtractRange.addSegment(ValueSegment(0 - seg.m_max, 0 - seg.m_min));
    }
    return add(subtractRange);
  }

  ValueRange mul(const ValueRange &b, unsigned width) const {
    return ValueRange(llvm::APInt::getNullValue(width),
                      llvm::APInt::getAllOnesValue(width));
  }
  ValueRange udiv(const ValueRange &b, unsigned width) const {
    return ValueRange(llvm::APInt::getNullValue(width),
                      llvm::APInt::getAllOnesValue(width));
  }
  ValueRange sdiv(const ValueRange &b, unsigned width) const {
    return ValueRange(llvm::APInt::getNullValue(width),
                      llvm::APInt::getAllOnesValue(width));
  }
  ValueRange urem(const ValueRange &b, unsigned width) const {
    return ValueRange(llvm::APInt::getNullValue(width),
                      llvm::APInt::getAllOnesValue(width));
  }
  ValueRange srem(const ValueRange &b, unsigned width) const {
    return ValueRange(llvm::APInt::getNullValue(width),
                      llvm::APInt::getAllOnesValue(width));
  }

  ValueRange zextOrTrunc(unsigned newWidth) const {
    ValueRange zextOrTruncRange;
    zextOrTruncRange.width = newWidth;
    for (const ValueSegment &seg : segments) {
      ValueSegment newSeg(seg.m_min.zextOrTrunc(newWidth),
                          seg.m_max.zextOrTrunc(newWidth));
      zextOrTruncRange.addSegment(newSeg);
    }
    return zextOrTruncRange;
  }
  ValueRange sextOrTrunc(unsigned newWidth) const {
    ValueRange sextOrTruncRange;
    sextOrTruncRange.width = newWidth;
    for (const ValueSegment &seg : segments) {
      ValueSegment newSeg(seg.m_min.sextOrTrunc(newWidth),
                          seg.m_max.sextOrTrunc(newWidth));
      sextOrTruncRange.addSegment(newSeg);
    }
    return sextOrTruncRange;
  }

  // use min() to get value if true (XXX should we add a method to
  // make code clearer?)
  bool isFixed() const noexcept {
    return segments.size() == 1 && segments.begin()->isFixed();
  }

  bool operator==(const ValueRange &b) const noexcept {
    return segments == b.segments && bitWidth() == b.bitWidth();
  }
  bool operator!=(const ValueRange &b) const noexcept { return !(*this == b); }

  bool mayEqual(const ValueRange &b) const { return this->intersects(b); }
  bool mayEqual(const llvm::APInt &b) const noexcept {
    return mayEqual(ValueRange(b));
  }

  bool mustEqual(const ValueRange &b) const noexcept {
    return isFixed() && b.isFixed() && segments == b.segments;
  }
  bool mustEqual(const llvm::APInt &b) const noexcept {
    return mustEqual(ValueRange(b));
  }

  // Getter for minimal value of this range from
  // ALL segments inside. This means, that min() from
  // { [0, 1], [2, 3] } is 1.
  llvm::APInt min() const noexcept {
    assert(!isEmpty() && "cannot get minimum of empty range");
    return segments.begin()->m_min;
  }

  // Getter for maximal value of this range from
  // ALL segments inside. This means, that max() from
  // { [0, 1], [2, 3] } is 3.
  llvm::APInt max() const noexcept {
    assert(!isEmpty() && "cannot get maximum of empty range");
    return (--segments.end())->m_max;
  }

  // TODO: do we still need bits here?
  llvm::APInt minSigned(unsigned bits) const {
    // if max allows sign bit to be set then it can be smallest value,
    // otherwise since the range is not empty, min cannot have a sign
    // bit

    llvm::APInt smallest = llvm::APInt::getSignedMinValue(bits);

    if (max().uge(smallest)) {
      return max().sext(bits);
    } else {
      return min();
    }
  }

  // Works like a sext instrution: if bits is less then
  // current width, then truncate expression; otherwise
  // extend it to bits.
  llvm::APInt maxSigned(unsigned bits) const {
    llvm::APInt smallest = llvm::APInt::getSignedMinValue(bits);

    // if max and min have sign bit then max is max, otherwise if only
    // max has sign bit then max is largest signed integer, otherwise
    // max is max

    if (min().ult(smallest) && max().uge(smallest)) {
      return smallest - 1;
    } else {
      // width are not equal here; if this width is shorter, then
      // we will return sign extended max, otherwise we need to find
      // signed max value of first n bits

      return max().sext(bits);
    }
  }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const ValueRange &vr) {
  vr.print(os);
  return os;
}

// XXX waste of space, rather have ByteValueRange
typedef ValueRange CexValueData;

class CexObjectData {
  /// possibleContents - An array of "possible" values for the object.
  ///
  /// The possible values is an inexact approximation for the set of values for
  /// each array location.
  SparseStorage<CexValueData> possibleContents;

  /// exactContents - An array of exact values for the object.
  ///
  /// The exact values are a conservative approximation for the set of values
  /// for each array location.
  SparseStorage<CexValueData> exactContents;

  CexObjectData(const CexObjectData &);  // DO NOT IMPLEMENT
  void operator=(const CexObjectData &); // DO NOT IMPLEMENT

public:
  CexObjectData(uint64_t size)
      : possibleContents(size,
                         ValueRange(llvm::APInt::getNullValue(CHAR_BIT),
                                    llvm::APInt::getAllOnesValue(CHAR_BIT))),
        exactContents(size,
                      ValueRange(llvm::APInt::getNullValue(CHAR_BIT),
                                 llvm::APInt::getAllOnesValue(CHAR_BIT))) {}

  const CexValueData getPossibleValues(size_t index) const {
    return possibleContents.load(index);
  }
  void setPossibleValues(size_t index, const CexValueData &values) {
    possibleContents.store(index, values);
  }
  void setPossibleValue(size_t index, const llvm::APInt &value) {
    possibleContents.store(index, CexValueData(value));
  }

  const CexValueData getExactValues(size_t index) const {
    return exactContents.load(index);
  }
  void setExactValues(size_t index, CexValueData values) {
    exactContents.store(index, values);
  }

  /// getPossibleValue - Return some possible value.
  llvm::APInt getPossibleValue(size_t index) const {
    CexValueData cvd = possibleContents.load(index);
    assert(!cvd.isEmpty() && "possible contents empty");
    const ValueRange::ValueSegment &seg = *cvd.segments.begin();
    return seg.m_min + (seg.m_max - seg.m_min).lshr(1);
  }
};

class CexRangeEvaluator : public ExprRangeEvaluator<ValueRange> {
public:
  std::map<const Array *, CexObjectData *> &objects;
  CexRangeEvaluator(std::map<const Array *, CexObjectData *> &_objects)
      : objects(_objects) {}

  ValueRange getInitialReadRange(const Array &array, ValueRange index) {
    // Check for a concrete read of a constant array.
    if (array.isConstantArray() && index.isFixed()) {
      if (isa<ConstantSource>(array.source) &&
          index.min().getZExtValue() < array.constantValues.size()) {
        return ValueRange(
            array.constantValues[index.min().getZExtValue()]->getAPValue());
      } else if (ref<ConstantWithSymbolicSizeSource>
                     constantWithSymbolicSizeSource =
                         dyn_cast<ConstantWithSymbolicSizeSource>(
                             array.source)) {
        return ValueRange(llvm::APInt(
            CHAR_BIT, constantWithSymbolicSizeSource->defaultValue));
      }
    }
    return ValueRange(llvm::APInt::getNullValue(CHAR_BIT),
                      llvm::APInt::getAllOnesValue(CHAR_BIT));
  }
};

class CexPossibleEvaluator : public ExprEvaluator {
protected:
  ref<Expr> getInitialValue(const Array &array, unsigned index) {
    // If the index is out of range, we cannot assign it a value, since that
    // value cannot be part of the assignment.
    ref<ConstantExpr> constantArraySize = dyn_cast<ConstantExpr>(array.size);

    if (!constantArraySize || index >= constantArraySize->getZExtValue()) {
      return ReadExpr::create(UpdateList(&array, 0),
                              ConstantExpr::alloc(index, array.getDomain()));
    }

    std::map<const Array *, CexObjectData *>::iterator it =
        objects.find(&array);
    return ConstantExpr::alloc(
        (it == objects.end()
             ? 127
             : it->second->getPossibleValue(index).getZExtValue()),
        array.getRange());
  }

public:
  std::map<const Array *, CexObjectData *> &objects;
  CexPossibleEvaluator(std::map<const Array *, CexObjectData *> &_objects)
      : objects(_objects) {}
};

class CexExactEvaluator : public ExprEvaluator {
protected:
  ref<Expr> getInitialValue(const Array &array, unsigned index) {
    // If the index is out of range, we cannot assign it a value, since that
    // value cannot be part of the assignment.
    ref<ConstantExpr> constantArraySize = dyn_cast<ConstantExpr>(array.size);

    if (!constantArraySize || index >= constantArraySize->getZExtValue()) {
      return ReadExpr::create(UpdateList(&array, 0),
                              ConstantExpr::alloc(index, array.getDomain()));
    }

    std::map<const Array *, CexObjectData *>::iterator it =
        objects.find(&array);
    if (it == objects.end())
      return ReadExpr::create(UpdateList(&array, 0),
                              ConstantExpr::alloc(index, array.getDomain()));

    CexValueData cvd = it->second->getExactValues(index);
    if (!cvd.isFixed())
      return ReadExpr::create(UpdateList(&array, 0),
                              ConstantExpr::alloc(index, array.getDomain()));

    return ConstantExpr::create(cvd.min().getZExtValue(), array.getRange());
  }

public:
  std::map<const Array *, CexObjectData *> &objects;
  CexExactEvaluator(std::map<const Array *, CexObjectData *> &_objects)
      : objects(_objects) {}
};

class CexData {
public:
  std::map<const Array *, CexObjectData *> objects;

  CexData(const CexData &);        // DO NOT IMPLEMENT
  void operator=(const CexData &); // DO NOT IMPLEMENT

public:
  CexData() {}
  ~CexData() {
    for (std::map<const Array *, CexObjectData *>::iterator
             it = objects.begin(),
             ie = objects.end();
         it != ie; ++it)
      delete it->second;
  }

  CexObjectData &getObjectData(const Array *A) {
    CexObjectData *&Entry = objects[A];

    ref<ConstantExpr> constantArraySize = dyn_cast<ConstantExpr>(A->size);

    // TODO: zero sized array?
    if (!constantArraySize) {
      constantArraySize = ConstantExpr::create(0, A->size->getWidth());
    }

    if (!Entry)
      Entry = new CexObjectData(constantArraySize->getZExtValue());

    return *Entry;
  }

  void propogatePossibleValue(ref<Expr> e, const llvm::APInt &value) {
    propogatePossibleValues(e, CexValueData(value));
  }

  void propogateExactValue(ref<Expr> e, const llvm::APInt &value) {
    propogateExactValues(e, CexValueData(value));
  }

  void propogatePossibleValues(ref<Expr> e, CexValueData range) {
    KLEE_DEBUG(llvm::errs() << "propogate: " << range << " for\n" << e << "\n");
    assert(range.bitWidth() == e->getWidth());
    // llvm::errs() << "propogate: " << range << " for\n" << e << "\n";

    switch (e->getKind()) {
    case Expr::Constant: {
      ref<ConstantExpr> CE = cast<ConstantExpr>(e);
      // TODO: wrong propagation may lead to UNSAT. Handle it.
      // assert(range.intersects(ValueRange(CE->getAPValue())) &&
      //        "Constant is out of range for propagation.");

      // rather a pity if the constant isn't in the range, but how can
      // we use this?
      break;
    }

      // Special

    case Expr::NotOptimized:
      break;

    case Expr::Read: {
      ReadExpr *re = cast<ReadExpr>(e);
      const Array *array = re->updates.root;
      CexObjectData &cod = getObjectData(array);

      // FIXME: This is imprecise, we need to look through the existing writes
      // to see if this is an initial read or not.
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(re->index)) {
        if (ref<ConstantExpr> constantArraySize =
                dyn_cast<ConstantExpr>(array->size)) {
          uint64_t index = CE->getZExtValue();

          // ReadExpr updateList
          // getInitialValue() -> array
          //
          // write concrete_idx ref<Expr>(range[a, b])
          //
          // CexValueData cvd = cod.getPossibleValues(index);

          // TODO: index in range
          if (index < constantArraySize->getZExtValue()) {
            // If the range is fixed, just set that; even if it conflicts with
            // the previous range it should be a better guess. TODO: leads to
            // UNSAT?

            // It would be nice to add ranges from new writes to this arrays to
            // make them once.
            // for (ref<UpdateNode> un = re->updates.head; un != nullptr;
            //      un = un->next) {
            //   // For writes at constant idxs it is ok. But what if index is
            //   // symbolic?
            // }

            if (range.isFixed()) {
              cod.setPossibleValue(index, range.min());
            } else {
              CexValueData cvd = cod.getPossibleValues(index);
              CexValueData tmp = cvd.set_intersection(range);

              if (!tmp.isEmpty())
                cod.setPossibleValues(index, tmp);
            }
          }
        } else {
          // XXX        fatal("XXX not implemented");
        }
      } else {
        // XXX        fatal("XXX not implemented");
      }
      break;
    }

    case Expr::Select: {
      SelectExpr *se = cast<SelectExpr>(e);
      ValueRange cond = evalRangeForExpr(se->cond);
      if (cond.isFixed()) {
        if (cond.min().getBoolValue()) {
          propogatePossibleValues(se->trueExpr, range);
        } else {
          propogatePossibleValues(se->falseExpr, range);
        }
      } else {
        // XXX imprecise... we have a choice here. One method is to
        // simply force both sides into the specified range (since the
        // condition is indetermined). This may lose in two ways, the
        // first is that the condition chosen may limit further
        // restrict the range in each of the children, however this is
        // less of a problem as the range will be a superset of legal
        // values. The other is if the condition ends up being forced
        // by some other constraints, then we needlessly forced one
        // side into the given range.
        //
        // The other method would be to force the condition to one
        // side and force that side into the given range. This loses
        // when we force the condition to an unsatisfiable value
        // (either because the condition cannot be that, or the
        // resulting range given that condition is not in the required
        // range).
        //
        // Currently we just force both into the range. A hybrid would
        // be to evaluate the ranges for each of the children... if
        // one of the ranges happens to already be a subset of the
        // required range then it may be preferable to force the
        // condition to that side.
        propogatePossibleValues(se->trueExpr, range);
        propogatePossibleValues(se->falseExpr, range);
      }
      break;
    }

      // XXX imprecise... the problem here is that extracting bits
      // loses information about what bits are connected across the
      // bytes. if a value can be 1 or 256 then either the top or
      // lower byte is 0, but just extraction loses this information
      // and will allow neither,one,or both to be 1.
      //
      // we can protect against this in a limited fashion by writing
      // the extraction a byte at a time, then checking the evaluated
      // value, isolating for that range, and continuing.
    case Expr::Concat: {
      ConcatExpr *ce = cast<ConcatExpr>(e);
      Expr::Width LSBWidth = ce->getLeft()->getWidth();
      Expr::Width MSBWidth = ce->getRight()->getWidth();
      propogatePossibleValues(ce->getLeft(), range.extract(0, LSBWidth));
      propogatePossibleValues(ce->getRight(),
                              range.extract(LSBWidth, LSBWidth + MSBWidth));
      break;
    }

    case Expr::Extract: {
      ExtractExpr *ee = cast<ExtractExpr>(e);
      // TODO:
      // propogatePossibleValues(ee->expr);
      // XXX
      break;
    }

      // Casting

      // Simply intersect the output range with the range of all possible
      // outputs and then truncate to the desired number of bits.

      // For ZExt this simplifies to just intersection with the possible input
      // range.
    case Expr::ZExt: {
      CastExpr *ce = cast<CastExpr>(e);
      unsigned inBits = ce->src->getWidth();
      unsigned outBits = ce->getWidth();

      // Intersect with range of same bitness and truncate
      // result to inBits (as llvm::APInt can not be compared
      // if they have different width).
      ValueRange input = range
                             .set_intersection(ValueRange(
                                 llvm::APInt::getNullValue(outBits),
                                 llvm::APInt::getLowBitsSet(outBits, inBits)))
                             .zextOrTrunc(inBits);
      propogatePossibleValues(ce->src, input);
      break;
    }
      // For SExt instead of doing the intersection we just take the output
      // range minus the impossible values. This is nicer since it is a single
      // interval.
    case Expr::SExt: {
      CastExpr *ce = cast<CastExpr>(e);
      unsigned inBits = ce->src->getWidth();
      unsigned outBits = ce->width;

      ValueRange input =
          range
              .set_difference(ValueRange(
                  llvm::APInt::getOneBitSet(outBits, inBits - 1),
                  (llvm::APInt::getAllOnesValue(outBits) -
                   llvm::APInt::getLowBitsSet(outBits, inBits - 1) - 1)))
              .zextOrTrunc(inBits);

      propogatePossibleValues(ce->src, input);
      break;
    }

      // Binary

    case Expr::Add: {
      BinaryExpr *be = cast<BinaryExpr>(e);
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(be->left)) {
        // FIXME: Why do we ever propogate empty ranges? It doesn't make
        // sense.

        // TODO: Add assertion. This should not happen.
        if (range.isEmpty())
          break;

        // [10 > a + b] <- propagate 1; [a + b] <- propagate [0, 9]
        // a <- [ [0, 1] ]              | b <- [1, 128]
        //
        // b <- [ [0, 9] , [255, 255] ] | a <-

        // C_0 + X \in [MIN, MAX) ==> X \in [MIN - C_0, MAX - C_0)
        CexValueData subtractedRange;
        subtractedRange.width = range.bitWidth();
        for (const ValueRange::ValueSegment &seg : range.segments) {
          ValueRange::ValueSegment newSeg(seg.m_min - CE->getAPValue(),
                                          seg.m_max - CE->getAPValue());
          if (newSeg.m_max.ult(newSeg.m_min)) {
            subtractedRange.addSegment(ValueRange::ValueSegment(
                llvm::APInt(range.bitWidth(), 0), newSeg.m_max));
            newSeg.m_max = llvm::APInt::getAllOnesValue(range.bitWidth());
          }
          subtractedRange.addSegment(newSeg);
          propogatePossibleValues(be->right, subtractedRange);
        }
      }
      break;
    }

    case Expr::And: {
      BinaryExpr *be = cast<BinaryExpr>(e);
      if (be->getWidth() == Expr::Bool) {
        if (range.isFixed()) {
          ValueRange left = evalRangeForExpr(be->left);
          ValueRange right = evalRangeForExpr(be->right);

          if (!range.min()) {
            if (left.mustEqual(llvm::APInt::getNullValue(be->getWidth())) ||
                right.mustEqual(llvm::APInt::getNullValue(be->getWidth()))) {
              // all is well
            } else {
              // XXX heuristic, which order

              propogatePossibleValue(
                  be->left, llvm::APInt::getNullValue(be->left->getWidth()));
              left = evalRangeForExpr(be->left);

              // see if that worked
              if (!left.mustEqual(llvm::APInt(be->left->getWidth(), 1))) {
                propogatePossibleValue(
                    be->right, llvm::APInt::getNullValue(be->left->getWidth()));
              }
            }
          } else {
            llvm::APInt leftAPIntOne = llvm::APInt(be->left->getWidth(), 1);
            if (!left.mustEqual(leftAPIntOne)) {
              propogatePossibleValue(be->left, leftAPIntOne);
            }
            llvm::APInt rightAPIntOne = llvm::APInt(be->right->getWidth(), 1);
            if (!right.mustEqual(rightAPIntOne)) {
              propogatePossibleValue(be->right, rightAPIntOne);
            }
          }
        }
      } else {
        // TODO: propogation of full ranges?
        // XXX
      }
      break;
    }

    case Expr::Or: {
      BinaryExpr *be = cast<BinaryExpr>(e);
      if (be->getWidth() == Expr::Bool) {
        if (range.isFixed()) {
          ValueRange left = evalRangeForExpr(be->left);
          ValueRange right = evalRangeForExpr(be->right);

          llvm::APInt zeroAPInt =
              llvm::APInt::getNullValue(be->left->getWidth());
          llvm::APInt oneAPInt = llvm::APInt(be->left->getWidth(), 1);
          if (range.min().getBoolValue()) {
            if (left.mustEqual(oneAPInt) || right.mustEqual(oneAPInt)) {
              // all is well
            } else {
              // XXX heuristic, which order?

              // force left to value we need
              propogatePossibleValue(be->left, oneAPInt);
              left = evalRangeForExpr(be->left);

              // see if that worked
              if (!left.mustEqual(oneAPInt))
                propogatePossibleValue(be->right, oneAPInt);
            }
          } else {
            if (!left.mustEqual(zeroAPInt))
              propogatePossibleValue(be->left, zeroAPInt);
            if (!right.mustEqual(zeroAPInt))
              propogatePossibleValue(be->right, zeroAPInt);
          }
        }
      } else {
        // TODO: propogation of ranges <= given range
        // XXX
      }
      break;
    }

    case Expr::Xor:
      break;

      // Comparison

    case Expr::Eq: {
      BinaryExpr *be = cast<BinaryExpr>(e);
      if (range.isFixed()) {
        if (ConstantExpr *CE = dyn_cast<ConstantExpr>(be->left)) {
          llvm::APInt value = CE->getAPValue();
          if (range.min().getBoolValue()) {
            propogatePossibleValue(be->right, value);
          } else {
            CexValueData range;
            if (value == 0) {
              range =
                  CexValueData(llvm::APInt(CE->getWidth(), 1),
                               llvm::APInt::getAllOnesValue(CE->getWidth()));
            } else {
              // TODO: use public interface, do not access private methods.
              range.width = CE->getWidth();
              range.addSegment(ValueRange::ValueSegment(
                  llvm::APInt::getNullValue(CE->getWidth()), value - 1));
              range.addSegment(ValueRange::ValueSegment(
                  value + 1, llvm::APInt::getAllOnesValue(CE->getWidth())));
            }
            propogatePossibleValues(be->right, range);
          }
        }
      }
      // else { TODO: propogate full range? }
      break;
    }

    case Expr::Not: {
      if (e->getWidth() == Expr::Bool && range.isFixed()) {
        propogatePossibleValue(
            e->getKid(0), llvm::APInt(e->getKid(0)->getWidth(), !range.min()));
      }
      break;
    }

    case Expr::Ult: {
      BinaryExpr *be = cast<BinaryExpr>(e);

      // XXX heuristic / lossy, what order if conflict

      if (range.isFixed()) {
        ValueRange left = evalRangeForExpr(be->left);
        ValueRange right = evalRangeForExpr(be->right);

        llvm::APInt maxValue =
            llvm::APInt::getAllOnesValue(be->right->getWidth());

        // XXX should deal with overflow (can lead to empty range)

        if (left.isFixed()) {
          if (!range.min().isNullValue()) {
            propogatePossibleValues(be->right,
                                    CexValueData(left.min() + 1, maxValue));
          } else {
            propogatePossibleValues(
                be->right,
                CexValueData(llvm::APInt::getNullValue(be->right->getWidth()),
                             left.min()));
          }
        } else if (right.isFixed()) {
          if (!range.min().isNullValue()) {
            propogatePossibleValues(
                be->left,
                CexValueData(llvm::APInt::getNullValue(be->right->getWidth()),
                             right.min() - 1));
          } else {
            propogatePossibleValues(be->left,
                                    CexValueData(right.min(), maxValue));
          }
        } else {
          // XXX ???
        }
      }
      break;
    }
    case Expr::Ule: {
      BinaryExpr *be = cast<BinaryExpr>(e);

      // XXX heuristic / lossy, what order if conflict

      if (range.isFixed()) {
        ValueRange left = evalRangeForExpr(be->left);
        ValueRange right = evalRangeForExpr(be->right);

        // XXX should deal with overflow (can lead to empty range)

        llvm::APInt maxValue =
            llvm::APInt::getAllOnesValue(be->right->getWidth());
        if (left.isFixed()) {
          if (range.min().getBoolValue()) {
            propogatePossibleValues(be->right,
                                    CexValueData(left.min(), maxValue));
          } else {
            propogatePossibleValues(
                be->right,
                CexValueData(llvm::APInt::getNullValue(be->right->getWidth()),
                             left.min() - 1));
          }
        } else if (right.isFixed()) {
          if (range.min().getBoolValue()) {
            propogatePossibleValues(
                be->left,
                CexValueData(llvm::APInt::getNullValue(be->right->getWidth()),
                             right.min()));
          } else {
            propogatePossibleValues(be->left,
                                    CexValueData(right.min() + 1, maxValue));
          }
        } else {
          // XXX ???
          // TODO: we can try to order it!
        }
      }
      break;
    }

    case Expr::Ne:
    case Expr::Ugt:
    case Expr::Uge:
    case Expr::Sgt:
    case Expr::Sge:
      assert(0 && "invalid expressions (uncanonicalized");

    default:
      break;
    }
  }

  void propogateExactValues(ref<Expr> e, CexValueData range) {
    switch (e->getKind()) {
    case Expr::Constant: {
      // FIXME: Assert that range contains this constant.
      break;
    }

      // Special

    case Expr::NotOptimized:
      break;

    case Expr::Read: {
      ReadExpr *re = cast<ReadExpr>(e);
      const Array *array = re->updates.root;
      CexObjectData &cod = getObjectData(array);
      CexValueData index = evalRangeForExpr(re->index);

      for (const auto *un = re->updates.head.get(); un; un = un->next.get()) {
        CexValueData ui = evalRangeForExpr(un->index);

        // If these indices can't alias, continue propogation
        if (!ui.mayEqual(index))
          continue;

        // Otherwise if we know they alias, propogate into the write value.
        if (ui.mustEqual(index) || re->index == un->index)
          propogateExactValues(un->value, range);
        return;
      }

      // We reached the initial array write, update the exact range if possible.
      if (index.isFixed()) {
        // FIXME: ???
        if (array->isConstantArray()) {
          // Verify the range.
          propogateExactValues(
              array->constantValues[index.min().getZExtValue()], range);
        } else {
          CexValueData cvd = cod.getExactValues(index.min().getZExtValue());
          if (range.min().ugt(cvd.min())) {
            assert(range.min().ule(cvd.max()));
            cvd = CexValueData(range.min(), cvd.max());
          }
          if (range.max().ult(cvd.max())) {
            assert(range.max().uge(cvd.min()));
            cvd = CexValueData(cvd.min(), range.max());
          }
          cod.setExactValues(index.min().getZExtValue(), cvd);
        }
      }
      break;
    }

    case Expr::Select: {
      break;
    }

    case Expr::Concat: {
      break;
    }

    case Expr::Extract: {
      break;
    }

      // Casting

    case Expr::ZExt: {
      break;
    }

    case Expr::SExt: {
      break;
    }

      // Binary

    case Expr::And: {
      break;
    }

    case Expr::Or: {
      break;
    }

    case Expr::Xor: {
      break;
    }

      // Comparison

    case Expr::Eq: {
      BinaryExpr *be = cast<BinaryExpr>(e);
      if (range.isFixed()) {
        if (ConstantExpr *CE = dyn_cast<ConstantExpr>(be->left)) {
          if (range.min().getBoolValue()) {
            // If the equality is true, then propogate the value.
            propogateExactValue(be->right, CE->getAPValue());
          } else {
            // If the equality is false and the comparison is of booleans,
            // then we can infer the value to propogate.
            if (be->right->getWidth() == Expr::Bool) {
              propogateExactValue(
                  be->right,
                  llvm::APInt(Expr::Bool, !CE->getAPValue().getBoolValue()));
            }
          }
        }
      }
      break;
    }

    // If a boolean not, and the result is known, propagate it
    case Expr::Not: {
      if (e->getWidth() == Expr::Bool && range.isFixed()) {
        llvm::APInt propValue =
            llvm::APInt(e->getWidth(), !range.min().getBoolValue());
        propogateExactValue(e->getKid(0), propValue);
      }
      break;
    }

    case Expr::Ult: {
      break;
    }

    case Expr::Ule: {
      break;
    }

    case Expr::Ne:
    case Expr::Ugt:
    case Expr::Uge:
    case Expr::Sgt:
    case Expr::Sge:
      assert(0 && "invalid expressions (uncanonicalized");

    default:
      break;
    }
  }

  ValueRange evalRangeForExpr(const ref<Expr> &e) {
    CexRangeEvaluator ce(objects);
    return ce.evaluate(e);
  }

  /// evaluate - Try to evaluate the given expression using a consistent fixed
  /// value for the current set of possible ranges.
  ref<Expr> evaluatePossible(ref<Expr> e) {
    return CexPossibleEvaluator(objects).visit(e);
  }

  ref<Expr> evaluateExact(ref<Expr> e) {
    return CexExactEvaluator(objects).visit(e);
  }

  void dump() {
    llvm::errs() << "-- propogated values --\n";
    for (std::map<const Array *, CexObjectData *>::iterator
             it = objects.begin(),
             ie = objects.end();
         it != ie; ++it) {
      const Array *A = it->first;
      ref<ConstantExpr> arrayConstantSize = dyn_cast<ConstantExpr>(A->size);
      if (!arrayConstantSize) {
        klee_warning("Cannot dump %s as it has symbolic size\n",
                     A->name.c_str());
      }

      CexObjectData *COD = it->second;

      llvm::errs() << A->name << "\n";
      llvm::errs() << "possible: [";
      for (unsigned i = 0; i < arrayConstantSize->getZExtValue(); ++i) {
        if (i)
          llvm::errs() << ", ";
        llvm::errs() << COD->getPossibleValues(i);
      }
      llvm::errs() << "]\n";
      llvm::errs() << "exact   : [";
      for (unsigned i = 0; i < arrayConstantSize->getZExtValue(); ++i) {
        if (i)
          llvm::errs() << ", ";
        llvm::errs() << COD->getExactValues(i);
      }
      llvm::errs() << "]\n";
    }
  }
};

/* *** */

class FastCexSolver : public IncompleteSolver {
public:
  FastCexSolver();
  ~FastCexSolver();

  IncompleteSolver::PartialValidity computeTruth(const Query &);
  bool computeValue(const Query &, ref<Expr> &result);
  bool computeInitialValues(const Query &,
                            const std::vector<const Array *> &objects,
                            std::vector<SparseStorage<unsigned char>> &values,
                            bool &hasSolution);
  bool computeValidity(const Query &, Solver::Validity &result);
  bool check(const Query &query, ref<SolverResponse> &result);
  bool computeValidityCore(const Query &query, ValidityCore &validityCore,
                           bool &hasSolution);
};

FastCexSolver::FastCexSolver() {}

FastCexSolver::~FastCexSolver() {}

/// propogateValues - Propogate value ranges for the given query and return the
/// propogation results.
///
/// \param query - The query to propogate values for.
///
/// \param cd - The initial object values resulting from the propogation.
///
/// \param checkExpr - Include the query expression in the constraints to
/// propogate.
///
/// \param isValid - If the propogation succeeds (returns true), whether the
/// constraints were proven valid or invalid.
///
/// \return - True if the propogation was able to prove validity or invalidity.
static bool propogateValues(const Query &query, CexData &cd, bool checkExpr,
                            bool &isValid) {
  for (const auto &constraint : query.constraints) {
    cd.propogatePossibleValue(constraint,
                              llvm::APInt(constraint->getWidth(), 1));
    cd.propogateExactValue(constraint, llvm::APInt(constraint->getWidth(), 1));
  }
  if (checkExpr) {
    cd.propogatePossibleValue(
        query.expr, llvm::APInt::getNullValue(query.expr->getWidth()));
    cd.propogateExactValue(query.expr,
                           llvm::APInt::getNullValue(query.expr->getWidth()));
  }

  KLEE_DEBUG(cd.dump());
  // cd.dump();

  // Check the result.
  bool hasSatisfyingAssignment = true;
  if (checkExpr) {
    if (!cd.evaluatePossible(query.expr)->isFalse())
      hasSatisfyingAssignment = false;

    // If the query is known to be true, then we have proved validity.
    if (cd.evaluateExact(query.expr)->isTrue()) {
      isValid = true;
      return true;
    }
  }

  for (const auto &constraint : query.constraints) {
    if (hasSatisfyingAssignment && !cd.evaluatePossible(constraint)->isTrue())
      hasSatisfyingAssignment = false;

    // If this constraint is known to be false, then we can prove anything, so
    // the query is valid.
    if (cd.evaluateExact(constraint)->isFalse()) {
      isValid = true;
      return true;
    }
  }

  if (hasSatisfyingAssignment) {
    isValid = false;
    return true;
  }

  return false;
}

IncompleteSolver::PartialValidity
FastCexSolver::computeTruth(const Query &query) {
  CexData cd;

  bool isValid;
  bool success = propogateValues(query, cd, true, isValid);

  if (!success) {
    return IncompleteSolver::None;
  }
  return isValid ? IncompleteSolver::MustBeTrue : IncompleteSolver::MayBeFalse;
}

bool FastCexSolver::computeValue(const Query &query, ref<Expr> &result) {
  CexData cd;

  bool isValid;
  bool success = propogateValues(query, cd, false, isValid);

  // Check if propogation wasn't able to determine anything.
  if (!success)
    return false;

  // FIXME: We don't have a way to communicate valid constraints back.
  if (isValid)
    return false;

  // Propogation found a satisfying assignment, evaluate the expression.
  ref<Expr> value = cd.evaluatePossible(query.expr);

  if (isa<ConstantExpr>(value)) {
    // FIXME: We should be able to make sure this never fails?
    result = value;
    return true;
  } else {
    return false;
  }
}

bool FastCexSolver::computeInitialValues(
    const Query &query, const std::vector<const Array *> &objects,
    std::vector<SparseStorage<unsigned char>> &values, bool &hasSolution) {
  CexData cd;

  bool isValid;
  bool success = propogateValues(query, cd, true, isValid);

  // Check if propogation wasn't able to determine anything.
  if (!success)
    return false;

  hasSolution = !isValid;
  if (!hasSolution)
    return true;

  // Propogation found a satisfying assignment, compute the initial values.
  for (unsigned i = 0; i != objects.size(); ++i) {
    const Array *array = objects[i];
    assert(array);
    SparseStorage<unsigned char> data;
    ref<ConstantExpr> arrayConstantSize =
        dyn_cast<ConstantExpr>(cd.evaluatePossible(array->size));
    assert(arrayConstantSize &&
           "Array of symbolic size had not receive value for size!");
    data.resize(arrayConstantSize->getZExtValue());

    for (unsigned i = 0; i < arrayConstantSize->getZExtValue(); i++) {
      ref<Expr> read = ReadExpr::create(
          UpdateList(array, 0), ConstantExpr::create(i, array->getDomain()));
      ref<Expr> value = cd.evaluatePossible(read);

      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(value)) {
        data.store(i, ((unsigned char)CE->getZExtValue(8)));
      } else {
        // FIXME: When does this happen?
        return false;
      }
    }

    values.push_back(data);
  }

  return true;
}

bool FastCexSolver::check(const Query &query, ref<SolverResponse> &result) {
  return false;
}

bool FastCexSolver::computeValidity(const Query &, Solver::Validity &result) {
  return false;
}

bool FastCexSolver::computeValidityCore(const Query &query,
                                        ValidityCore &validityCore,
                                        bool &hasSolution) {
  return false;
}

Solver *klee::createFastCexSolver(Solver *s) {
  return new Solver(new StagedSolverImpl(new FastCexSolver(), s));
}
