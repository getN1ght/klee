//===-- TargetForest.h ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Class to represent prefix tree of Targets
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_TARGETFOREST_H
#define KLEE_TARGETFOREST_H

#include "Target.h"
#include "klee/ADT/Ref.h"
#include "klee/Module/KModule.h"
#include "klee/Module/Locations.h"

#include <unordered_map>

namespace klee {

class TargetForest {
private:
  class Layer {
    using InternalLayer = std::unordered_map<ref<Target>, ref<Layer>, TargetHash, TargetCmp>;
    InternalLayer forest;

    explicit Layer(const InternalLayer &forest) : forest(forest) {}
    void unionWith(const Layer *other);

    static Layer *pathForestToTargetForest(Layer *self, PathForest *pathForest, std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2Targets);

  public:
    using iterator = InternalLayer::iterator;

    /// @brief Required by klee::ref-managed objects
    class ReferenceCounter _refCount;

    explicit Layer() {}
    Layer(PathForest *pathForest, std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2Targets);

    iterator find(ref<Target> b) { return forest.find(b); }
    iterator begin() { return forest.begin(); }
    iterator end() { return forest.end(); }
    void insert(ref<Target> loc, Layer *nextLayer) { forest.insert(std::make_pair(loc, nextLayer)); }
    bool empty() const { return forest.empty(); }
    size_t size() const { return forest.size(); }
    Layer *replaceChildWith(ref<Target> child, const Layer *other) const;
    Layer *removeChild(ref<Target> child) const;
    Layer *addChild(ref<Target> child) const;
    bool allNodesRefCountOne() const;
    void dump() const;
  };

  ref<Layer> forest;

  bool allNodesRefCountOne() const;

public:
  class History {
  private:
    unsigned hashValue;

  public:
    const ref<Target> target;
    const ref<History> visitedTargets;

    explicit History(ref<Target> _target, ref<History> _visitedTargets)
        : target(_target), visitedTargets(_visitedTargets) {
      computeHash();
    }
    explicit History(ref<Target> _target) : History(_target, nullptr) {}
    explicit History() : History(nullptr) {}

    ref<History> add(ref<Target> _target) { return new History(_target, this); }

    unsigned hash() const { return hashValue; }

    int compare(const History &h) const;

    void computeHash() {
      unsigned res = 0;
      if (target) {
        res = target->hash() * Expr::MAGIC_HASH_CONSTANT;
      }
      if (visitedTargets) {
        res ^= visitedTargets->hash() * Expr::MAGIC_HASH_CONSTANT;
      }
      hashValue = res;
    }

    /// @brief Required by klee::ref-managed objects
    class ReferenceCounter _refCount;
  };

private:
  ref<History> history;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;
  unsigned getDebugReferenceCount() { return forest->_refCount.getCount(); }
  void debugStepToRandomLoc();

  TargetForest(ref<Layer> layer) : forest(layer), history(new History()) {}
  TargetForest() : TargetForest(new Layer()) {}
  TargetForest(PathForest *pathForest, std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2Targets);

  bool empty() const { return forest->empty(); }
  Layer::iterator begin() const { return forest->begin(); }
  Layer::iterator end() const { return forest->end(); }
  bool contains(ref<Target> b) { return forest->find(b) != forest->end(); }

  /// @brief Number of children of this layer (immediate successors)
  size_t successorCount() const { return forest->size(); }

  void stepTo(ref<Target>);
  void add(ref<Target>);
  void remove(ref<Target>);
  const ref<History> getHistory() { return history; };
  void dump() const;
};

struct TargetsHash {
  unsigned operator()(const ref<TargetForest::History> &t) const {
    return t->hash();
  }
};

struct TargetsCmp {
  bool operator()(const ref<TargetForest::History> &a,
                  const ref<TargetForest::History> &b) const {
    return a == b;
  }
};

} // End klee namespace

#endif /* KLEE_TARGETFOREST_H */
