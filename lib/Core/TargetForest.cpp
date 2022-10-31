//===-- TargetForest.cpp --------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TargetForest.h"
#include "klee/Module/KModule.h"
#include "klee/Module/KInstruction.h"

using namespace klee;
using namespace llvm;


TargetForest::TargetForest(const std::vector<ResolvedLocations> &paths, const std::unordered_map<KBlock *, Target *> &block2target) : TargetForest() {
  for (const auto &path : paths)
    addPath(path, block2target);
}

void TargetForest::addPath(const ResolvedLocations &rl, const std::unordered_map<KBlock *, Target *> &block2target) {
  const auto &path = rl.locations;
  auto n = path.size();
  if (n == 0)
    return;
  std::vector<std::tuple<size_t, bool, TargetForest*> > q;
  q.emplace_back(0, true, this);
  while (!q.empty()) {
    auto i = std::get<0>(q.back());
    bool reading = std::get<1>(q.back());
    auto next = std::get<2>(q.back());
    q.pop_back();
    TargetForest::Layer *current = next->forest.get();
    TargetForest *nextForQueue = nullptr;
    auto &loc_basket = path[i].blocks;
    for (auto loc : loc_basket) {
      auto target = block2target.at(loc);
      bool readingForQueue = reading;
      if (readingForQueue) {
        auto res = current->find(target);
        if (res != current->end()) {
          nextForQueue = res->second.get();
        } else
          readingForQueue = false;
      }
      if (!readingForQueue) {
        nextForQueue = new TargetForest();
        current->insert(target, nextForQueue);
      }
      if (i + 1 < n)
        q.emplace_back(i + 1, readingForQueue, nextForQueue);
    }
  }
  assert(allNodesRefCountOne());
}

void TargetForest::Layer::unionWith(const TargetForest::Layer *other) {
  for (const auto &kv : *other->forest) {
    auto it = forest->find(kv.first);
    if (it == forest->end()) {
      forest->insert(it, kv);
      continue;
    }
    it->second->forest->unionWith(kv.second->forest.get());
  }
}

TargetForest::Layer *TargetForest::Layer::replaceChildWith(Target *const child, const TargetForest::Layer *other) const {
  auto layer = new InternalLayer(*forest);
  layer->erase(child);
  auto result = new Layer(layer);
  result->unionWith(other);
  return result;
}

void TargetForest::stepTo(Target *loc) {
  auto res = forest->find(loc);
  if (res == forest->end()) {
    return;
  }
  forest = forest->replaceChildWith(loc, res->second->forest.get());
}

void TargetForest::debugStepToRandomLoc() {
  forest = forest->begin()->second->forest;
}

bool TargetForest::allNodesRefCountOne() const {
  bool all = true;
  for (const auto &it : *forest) {
    all &= it.second->_refCount.getCount() == 1;
    assert(all);
    all &= it.second->allNodesRefCountOne();
  }
  return all;
}
