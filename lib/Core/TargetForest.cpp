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


TargetForest::TargetForest(const std::vector<Locations *> &paths,
                           std::unordered_map<klee::Location *, std::unordered_set<klee::KBlock *> *> &loc2blocks,
                           std::unordered_map<klee::KBlock *, std::unordered_map<klee::ReachWithError, klee::ref<klee::Target>> *> &block2targets) : TargetForest() {
  for (const auto &path : paths)
    addPath(path, loc2blocks, block2targets);
}

void TargetForest::addPath(Locations *rl,
                           std::unordered_map<klee::Location *, std::unordered_set<klee::KBlock *> *> &loc2blocks,
                           std::unordered_map<klee::KBlock *, std::unordered_map<klee::ReachWithError, klee::ref<klee::Target>> *> &block2targets) {
  const auto &path = rl->path;
  auto errorType = rl->targetError();
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
    auto loc = path[i];
    auto loc_basket = loc2blocks[loc];
    auto error = i == n - 1 ? errorType : ReachWithError::None;
    for (auto block : *loc_basket) {
      auto target = (*block2targets[block])[error];
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
  if (other->forest.empty())
    return;
  for (const auto &kv : other->forest) {
    auto it = forest.find(kv.first);
    if (it == forest.end()) {
      forest.insert(std::make_pair(kv.first, kv.second));
      continue;
    }
    auto layer = new Layer(it->second->forest->forest);
    layer->unionWith(kv.second->forest.get());
    it->second->forest = layer;
  }
}

TargetForest::Layer *TargetForest::Layer::removeChild(ref<Target> child) const {
  auto result = new Layer(forest);
  result->forest.erase(child);
  return result;
}

TargetForest::Layer *TargetForest::Layer::addChild(ref<Target> child) const {
  auto result = new Layer(forest);
  result->forest.insert({child, new TargetForest()});
  return result;
}

TargetForest::Layer *TargetForest::Layer::replaceChildWith(ref<Target> const child, const TargetForest::Layer *other) const {
  auto result = removeChild(child);
  result->unionWith(other);
  return result;
}

void TargetForest::stepTo(ref<Target> loc) {
  if (forest->empty())
    return;
  auto res = forest->find(loc);
  if (res == forest->end()) {
    return;
  }
  forest = forest->replaceChildWith(loc, res->second->forest.get());
}

void TargetForest::add(ref<Target> target) {
  auto res = forest->find(target);
  if (res != forest->end()) {
    return;
  }
  forest = forest->addChild(target);
}

void TargetForest::remove(ref<Target> target) {
  auto res = forest->find(target);
  if (res == forest->end()) {
    return;
  }
  forest = forest->removeChild(target);
}

void TargetForest::dump() const {
  llvm::errs() << "THE LAYER:\n";
  for (const auto &kv : *forest) {
    llvm::errs() << kv.first->toString() << "\n";
  }
  llvm::errs() << "-----------------------\n";
  for (const auto &kv : *forest) {
    kv.second->dump();
  }
  llvm::errs() << "-----------------------\n";
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
