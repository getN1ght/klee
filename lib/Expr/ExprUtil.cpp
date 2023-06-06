//===-- ExprUtil.cpp ------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Expr/ExprUtil.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprHashMap.h"
#include "klee/Expr/ExprVisitor.h"

#include <set>

using namespace klee;

///

namespace klee {

class ObjectFinder : public ExprVisitor {
protected:
  bool findOnlySymbolicObjects;

  Action visitRead(const ReadExpr &re) {
    const UpdateList &ul = re.updates;

    visit(ul.root->getSize());
    // XXX should we memo better than what ExprVisitor is doing for us?
    for (const auto *un = ul.head.get(); un; un = un->next.get()) {
      visit(un->index);
      visit(un->value);
    }

    if (!findOnlySymbolicObjects || ul.root->isSymbolicArray())
      if (results.insert(ul.root).second)
        objects.push_back(ul.root);

    return Action::doChildren();
  }

public:
  std::set<const Array *> results;
  std::vector<const Array *> &objects;

  ObjectFinder(std::vector<const Array *> &_objects,
               bool _findOnlySymbolicObjects = false)
      : findOnlySymbolicObjects(_findOnlySymbolicObjects), objects(_objects) {}
};

class SymbolicObjectFinder : public ObjectFinder {
public:
  SymbolicObjectFinder(std::vector<const Array *> &_objects)
      : ObjectFinder(_objects, true) {}
};

ExprVisitor::Action ConstantArrayFinder::visitRead(const ReadExpr &re) {
  const UpdateList &ul = re.updates;

  visit(ul.root->getSize());
  // FIXME should we memo better than what ExprVisitor is doing for us?
  for (const auto *un = ul.head.get(); un; un = un->next.get()) {
    visit(un->index);
    visit(un->value);
  }

  if (ul.root->isConstantArray()) {
    results.insert(ul.root);
  }

  return Action::doChildren();
}
} // namespace klee

template <typename InputIterator>
void klee::findSymbolicObjects(InputIterator begin, InputIterator end,
                               std::vector<const Array *> &results) {
  SymbolicObjectFinder of(results);
  for (; begin != end; ++begin)
    of.visit(*begin);
}

void klee::findSymbolicObjects(ref<Expr> e,
                               std::vector<const Array *> &results) {
  findSymbolicObjects(&e, &e + 1, results);
}

template <typename InputIterator>
void klee::findObjects(InputIterator begin, InputIterator end,
                       std::vector<const Array *> &results) {
  ObjectFinder of(results);
  for (; begin != end; ++begin)
    of.visit(*begin);
}

void klee::findObjects(ref<Expr> e, std::vector<const Array *> &results) {
  findObjects(&e, &e + 1, results);
}

typedef std::vector<ref<Expr>>::iterator A;
template void klee::findSymbolicObjects<A>(A, A, std::vector<const Array *> &);

typedef std::set<ref<Expr>>::iterator B;
template void klee::findSymbolicObjects<B>(B, B, std::vector<const Array *> &);

typedef ExprHashSet::iterator C;
template void klee::findSymbolicObjects<C>(C, C, std::vector<const Array *> &);

typedef std::vector<ref<Expr>>::iterator A;
template void klee::findObjects<A>(A, A, std::vector<const Array *> &);

typedef std::vector<ref<Expr>>::const_iterator cA;
template void klee::findObjects<cA>(cA, cA, std::vector<const Array *> &);

typedef std::set<ref<Expr>>::iterator B;
template void klee::findObjects<B>(B, B, std::vector<const Array *> &);

typedef ExprHashSet::iterator C;
template void klee::findObjects<C>(C, C, std::vector<const Array *> &);

bool klee::isReadFromSymbolicArray(ref<Expr> e) {
  if (auto read = dyn_cast<ReadExpr>(e)) {
    return !read->updates.root->isConstantArray();
  }
  if (auto concat = dyn_cast<ConcatExpr>(e)) {
    for (size_t i = 0; i < concat->getNumKids(); i++) {
      if (!isReadFromSymbolicArray(concat->getKid(i))) {
        return false;
      }
    }
    return true;
  }
  return false;
}

ref<Expr>
klee::createNonOverflowingSumExpr(const std::vector<ref<Expr>> &terms) {
  if (terms.empty()) {
    return ConstantExpr::create(0, Expr::Bool);
  }

  Expr::Width termWidth = terms.front()->getWidth();
  uint64_t overflowBits = sizeof(unsigned long long) * CHAR_BIT - 1 -
                          __builtin_clzll(terms.size() + 1);

  ref<Expr> sum = ConstantExpr::create(0, termWidth + overflowBits);
  for (const ref<Expr> &expr : terms) {
    assert(termWidth == expr->getWidth());
    sum =
        AddExpr::create(sum, ZExtExpr::create(expr, termWidth + overflowBits));
  }
  return sum;
}
