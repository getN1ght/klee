//===-- ExprUtil.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_EXPRUTIL_H
#define KLEE_EXPRUTIL_H

#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprVisitor.h"

#include <set>
#include <unordered_set>
#include <vector>

namespace klee {
class Array;
class Expr;
class ReadExpr;
template <typename T> class ref;

/// Find all ReadExprs used in the expression DAG. If visitUpdates
/// is true then this will including those reachable by traversing
/// update lists. Note that this may be slow and return a large
/// number of results.
template <typename T>
void findReads(ref<Expr> e, bool visitUpdates, std::vector<ref<T>> &results) {
  // Invariant: \forall_{i \in stack} !i.isConstant() && i \in visited
  std::vector<ref<Expr>> stack;
  ExprHashSet visited;
  std::set<const UpdateNode *> updates;

  if (!isa<ConstantExpr>(e)) {
    visited.insert(e);
    stack.push_back(e);
  }

  while (!stack.empty()) {
    ref<Expr> top = stack.back();
    stack.pop_back();

    if (T *t = dyn_cast<T>(top)) {
      results.push_back(t);
      // FIXME: temporary. Rewrite.
      if (!visitUpdates) {
        continue;
      }
    }

    if (ReadExpr *re = dyn_cast<ReadExpr>(top)) {
      // We memoized so can just add to list without worrying about
      // repeats.

      if (!isa<ConstantExpr>(re->index) && visited.insert(re->index).second)
        stack.push_back(re->index);

      if (re->updates.root->getSize() &&
          visited.insert(re->updates.root->getSize()).second) {
        stack.push_back(re->updates.root->getSize());
      }

      if (visitUpdates) {
        // XXX this is probably suboptimal. We want to avoid a potential
        // explosion traversing update lists which can be quite
        // long. However, it seems silly to hash all of the update nodes
        // especially since we memoize all the expr results anyway. So
        // we take a simple approach of memoizing the results for the
        // head, which often will be shared among multiple nodes.
        if (updates.insert(re->updates.head.get()).second) {
          for (const auto *un = re->updates.head.get(); un;
               un = un->next.get()) {
            if (!isa<ConstantExpr>(un->index) &&
                visited.insert(un->index).second)
              stack.push_back(un->index);
            if (!isa<ConstantExpr>(un->value) &&
                visited.insert(un->value).second)
              stack.push_back(un->value);
          }
        }
      }
    } else if (!isa<ConstantExpr>(top)) {
      Expr *e = top.get();
      for (unsigned i = 0; i < e->getNumKids(); i++) {
        ref<Expr> k = e->getKid(i);
        if (!isa<ConstantExpr>(k) && visited.insert(k).second)
          stack.push_back(k);
      }
    }
  }
}

/// Return a list of all unique symbolic objects referenced by the given
/// expression.
void findSymbolicObjects(ref<Expr> e, std::vector<const Array *> &results);

/// Return a list of all unique symbolic objects referenced by the
/// given expression range.
template <typename InputIterator>
void findSymbolicObjects(InputIterator begin, InputIterator end,
                         std::vector<const Array *> &results);

template <typename InputIterator>
void findObjects(InputIterator begin, InputIterator end,
                 std::vector<const Array *> &results);

void findObjects(ref<Expr> e, std::vector<const Array *> &results);

bool isReadFromSymbolicArray(ref<Expr> e);

ref<Expr> createNonOverflowingSumExpr(const std::vector<ref<Expr>> &terms);

class ConstantArrayFinder : public ExprVisitor {
protected:
  ExprVisitor::Action visitRead(const ReadExpr &re);

public:
  std::set<const Array *> results;
};
} // namespace klee

#endif /* KLEE_EXPRUTIL_H */
