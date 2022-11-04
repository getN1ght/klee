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


TargetForest::Layer *TargetForest::Layer::pathForestToTargetForest(TargetForest::Layer *self, PathForest *pathForest, std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2Targets) {
  if (pathForest == nullptr)
    return self;
  for (auto &p : pathForest->layer) {
    auto it = loc2Targets.find(p.first);
    if (it == loc2Targets.end()) {
      klee_warning("Cannot resolve location %s. It will be fixed when input will be based on llvm basic blocks instead of code line numbers.", p.first->toString().c_str());
      continue;
    }
    auto targets = it->second;
    for (auto &target : *targets) {
      auto next = new TargetForest::Layer();
      self->insert(target, pathForestToTargetForest(next, p.second, loc2Targets));
    }
  }
  return self;
}

TargetForest::Layer::Layer(PathForest *pathForest, std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2Targets) : Layer() {
  pathForestToTargetForest(this, pathForest, loc2Targets);
}

TargetForest::TargetForest(PathForest *pathForest, std::unordered_map<LocatedEvent *, std::set<ref<Target> > *> &loc2Targets)
  : TargetForest(new TargetForest::Layer(pathForest, loc2Targets)) {}

void TargetForest::Layer::unionWith(const TargetForest::Layer *other) {
  if (other->forest.empty())
    return;
  for (const auto &kv : other->forest) {
    auto it = forest.find(kv.first);
    if (it == forest.end()) {
      forest.insert(std::make_pair(kv.first, kv.second));
      continue;
    }
    auto layer = new Layer(it->second->forest);
    layer->unionWith(kv.second.get());
    it->second = layer;
  }
}

TargetForest::Layer *TargetForest::Layer::removeChild(ref<Target> child) const {
  auto result = new Layer(forest);
  result->forest.erase(child);
  return result;
}

TargetForest::Layer *TargetForest::Layer::addChild(ref<Target> child) const {
  auto result = new Layer(forest);
  result->forest.insert({child, new Layer()});
  return result;
}

TargetForest::Layer *TargetForest::Layer::replaceChildWith(ref<Target> const child, const TargetForest::Layer *other) const {
  auto result = removeChild(child);
  result->unionWith(other);
  return result;
}

bool TargetForest::Layer::allNodesRefCountOne() const {
  bool all = true;
  for (const auto &it : forest) {
    all &= it.second->_refCount.getCount() == 1;
    assert(all);
    all &= it.second->allNodesRefCountOne();
  }
  return all;
}

void TargetForest::Layer::dump(unsigned n) const {
  llvm::errs() << "THE " << n << " LAYER:\n";
  for (const auto &kv : forest) {
    llvm::errs() << kv.first->toString() << "\n";
  }
  llvm::errs() << "-----------------------\n";
  if (!forest.empty()) {
    for (const auto &kv : forest) {
      kv.second->dump(n + 1);
    }
    llvm::errs() << "++++++++++++++++++++++\n";
  }
}


int TargetForest::History::compare(const History &h) const {
  if (this == &h)
    return 0;

  if (target && h.target) {
    if (target != h.target)
      return (target < h.target) ? -1 : 1;
  } else {
    return h.target ? -1 : (target ? 1 : 0);
  }

  if (visitedTargets && h.visitedTargets) {
    if (h.visitedTargets != h.visitedTargets)
      return (visitedTargets < h.visitedTargets) ? -1 : 1;
  } else {
    return h.visitedTargets ? -1 : (visitedTargets ? 1 : 0);
  }

  return 0;
}

void TargetForest::History::dump() const {
  if (target) {
    llvm::errs() << target->toString() << "\n";
  }
  else {
    llvm::errs() << "end.\n";
    assert(!visitedTargets);
    return;
  }
  if (visitedTargets)
    visitedTargets->dump();

}

void TargetForest::stepTo(ref<Target> loc) {
  if (forest->empty())
    return;
  auto res = forest->find(loc);
  if (res == forest->end()) {
    return;
  }
  history = history->add(loc);
  if (loc->shouldFailOnThisTarget()) {
    forest = new Layer();
  } else {
    forest = forest->replaceChildWith(loc, res->second.get());
  }
}

void TargetForest::add(ref<Target> target) {
  auto res = forest->find(target);
  if (res != forest->end()) {
    return;
  }
  forest = forest->addChild(target);;
}

void TargetForest::remove(ref<Target> target) {
  auto res = forest->find(target);
  if (res == forest->end()) {
    return;
  }
  forest = forest->removeChild(target);
}

void TargetForest::dump() const {
  llvm::errs() << "History:\n";
  history->dump();
  llvm::errs() << "Forest:\n";
  forest->dump(1);
}

void TargetForest::debugStepToRandomLoc() {
  forest = forest->begin()->second;
}

bool TargetForest::allNodesRefCountOne() const {
  return forest->allNodesRefCountOne();
}
