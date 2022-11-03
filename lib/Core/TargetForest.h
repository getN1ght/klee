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

#include "klee/Module/KModule.h"
#include "klee/Module/Locations.h"
#include "klee/ADT/Ref.h"
#include "Target.h"

#include <unordered_map>


namespace klee {

class TargetForest {
  class Layer {
    using InternalLayer = std::unordered_map<ref<Target>, ref<TargetForest>, TargetHash, TargetCmp>;
    InternalLayer forest;

    explicit Layer(const InternalLayer &forest) : forest(forest) {}
    void unionWith(const Layer *other);

  public:
    using iterator = InternalLayer::iterator;

    /// @brief Required by klee::ref-managed objects
    class ReferenceCounter _refCount;

    explicit Layer() {}

    iterator find(ref<Target> b) { return forest.find(b); }
    iterator begin() { return forest.begin(); }
    iterator end() { return forest.end(); }
    void insert(ref<Target> loc, TargetForest *nextLayer) { forest.insert(std::make_pair(loc, nextLayer)); }
    bool empty() const { return forest.empty(); }
    size_t size() const { return forest.size(); }
    Layer *replaceChildWith(ref<Target> child, const Layer *other) const;
    Layer *removeChild(ref<Target> child) const;
    Layer *addChild(ref<Target> child) const;
  };
  ref<Layer> forest;

  /// @brief Add all path combinations from resolved locations
  void addPath(Locations *path, std::unordered_map<klee::Location *, std::unordered_set<klee::KBlock *> *> &loc2blocks, std::unordered_map<klee::KBlock *, std::unordered_map<klee::ReachWithError, klee::ref<klee::Target>> *> &block2targets);

  bool allNodesRefCountOne() const;
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;
  unsigned getDebugReferenceCount() {return forest->_refCount.getCount();}
  void debugStepToRandomLoc();

  TargetForest() : forest(new Layer()) {}
  TargetForest(const std::vector<Locations *> &paths, std::unordered_map<klee::Location *, std::unordered_set<klee::KBlock *> *> &loc2blocks, std::unordered_map<klee::KBlock *, std::unordered_map<klee::ReachWithError, klee::ref<klee::Target>> *> &block2targets);

  bool empty() const { return forest->empty(); }
  Layer::iterator begin() const { return forest->begin(); }
  Layer::iterator end() const { return forest->end(); }
  bool contains(ref<Target> b) { return forest->find(b) != forest->end(); }

  /// @brief Number of children of this layer (immediate successors)
  size_t successorCount() const { return forest->size(); }

  void stepTo(ref<Target> );
  void add(ref<Target> );
  void remove(ref<Target> );
  void dump() const;
};

} // End klee namespace

#endif /* KLEE_TARGETFOREST_H */
