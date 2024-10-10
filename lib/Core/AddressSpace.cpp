//===-- AddressSpace.cpp --------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AddressSpace.h"

#include "ExecutionState.h"
#include "Memory.h"
#include "TimingSolver.h"

#include "klee/Expr/ArrayExprVisitor.h"
#include "klee/Expr/Expr.h"
#include "klee/Expr/ExprUtil.h"
#include "klee/Module/KType.h"
#include "klee/Statistics/TimerStatIncrementer.h"

#include "CoreStats.h"
#include <algorithm>
#include <llvm-14/llvm/Support/Casting.h>

namespace klee {
llvm::cl::OptionCategory
    PointerResolvingCat("Pointer resolving options",
                        "These options impact pointer resolving.");

llvm::cl::opt<bool> SkipNotSymbolicObjects(
    "skip-not-symbolic-objects", llvm::cl::init(false),
    llvm::cl::desc("Set pointers only on symbolic objects, "
                   "use only with timestamps (default=false)"),
    llvm::cl::cat(PointerResolvingCat));

llvm::cl::opt<bool> SkipNotLazyInitialized(
    "skip-not-lazy-initialized", llvm::cl::init(false),
    llvm::cl::desc("Set pointers only on lazy initialized objects, "
                   "use only with timestamps (default=false)"),
    llvm::cl::cat(PointerResolvingCat));

llvm::cl::opt<bool> SkipLocal(
    "skip-local", llvm::cl::init(false),
    llvm::cl::desc(
        "Do not set symbolic pointers on local objects (default=false)"),
    llvm::cl::cat(PointerResolvingCat));

llvm::cl::opt<bool> SkipGlobal(
    "skip-global", llvm::cl::init(false),
    llvm::cl::desc(
        "Do not set symbolic pointers on global objects (default=false)"),
    llvm::cl::cat(PointerResolvingCat));

llvm::cl::opt<bool> UseTimestamps(
    "use-timestamps", llvm::cl::init(true),
    llvm::cl::desc("Set symbolic pointers only to objects created before those "
                   "pointers were created (default=true)"),
    llvm::cl::cat(PointerResolvingCat));
} // namespace klee

using namespace klee;

///

void AddressSpace::bindObject(const MemoryObject *mo, ObjectState *os) {
  assert(os->copyOnWriteOwner == 0 && "object already has owner");
  os->copyOnWriteOwner = cowKey;
  idsToObjects = idsToObjects.replace({mo->id, mo});
  objects = objects.replace(std::make_pair(mo, os));
}

void AddressSpace::unbindObject(const MemoryObject *mo) {
  idsToObjects = idsToObjects.remove(mo->id);
  objects = objects.remove(mo);
}

ResolveResult<ObjectPair>
AddressSpace::findObject(const MemoryObject *mo) const {
  const auto res = objects.lookup(mo);
  return res ? ResolveResult<ObjectPair>::createOk(
                   {res->first, res->second.get()})
             : ResolveResult<ObjectPair>::createNone();
}

ResolveResult<ObjectPair>
AddressSpace::lazyInitializeObject(const MemoryObject *mo) const {
  auto valueArray = mo->content;
  assert(valueArray != nullptr);

  const Array *baseArray = Array::create(
      mo->getSizeExpr(),
      SourceBuilder::makeSymbolic(valueArray->source->toString() + "_base", 0),
      Expr::Int32, Context::get().getPointerWidth());

  return ResolveResult<ObjectPair>::createOk(ObjectPair(
      mo, new ObjectState(mo, ObjectStateContent(valueArray, baseArray),
                          mo->type)));
}

ObjectState *AddressSpace::getWriteable(ObjectPair objPair) {
  return getWriteable(objPair.first, objPair.second);
}

ObjectState *AddressSpace::getWriteable(const MemoryObject *mo,
                                        const ObjectState *os) {
  // If this address space owns they object, return it
  if (cowKey == os->copyOnWriteOwner)
    return const_cast<ObjectState *>(os);

  // Add a copy of this object state that can be updated
  ref<ObjectState> newObjectState(new ObjectState(*os));
  bindObject(mo, newObjectState.get());
  return newObjectState.get();
}

///

ResolveResult<ObjectPair>
AddressSpace::resolveOne(ref<ConstantPointerExpr> pointer, KType *) const {
  auto base = pointer->getConstantBase()->getZExtValue();
  auto address = pointer->getConstantValue()->getZExtValue();

  if (idsToObjects.count(base) == 0) {
    return ResolveResult<ObjectPair>::createNone();
  }

  auto object = idsToObjects.at(base);
  auto resolution = findObject(idsToObjects.at(base));
  assert(resolution.isOk());

  // Get constant value of size
  auto objectConstantSizeExpr = dyn_cast<ConstantExpr>(object->getSizeExpr());
  if (objectConstantSizeExpr == nullptr) {
    return resolution;
  }
  auto objectConstantSizeValue = objectConstantSizeExpr->getZExtValue();

  // Get constant value of address
  auto objectConstantAddressExpr =
      dyn_cast<ConstantExpr>(object->getBaseExpr()->getValue());
  if (objectConstantAddressExpr == nullptr) {
    return ResolveResult<ObjectPair>::createUnknown(resolution.get());
  }
  uint64_t objectConstantAddressValue =
      objectConstantAddressExpr->getZExtValue();

  // Check if the provided address is between start and end of the object
  // [mo->address, mo->address + mo->size) or the object is a 0 - sized
  // object.
  if ((objectConstantSizeValue == 0 && address == objectConstantAddressValue) ||
      address - objectConstantAddressValue < objectConstantSizeValue) {
    return resolution;
  }
  return ResolveResult<ObjectPair>::createNone();
}

class ResolvePredicate {
  bool useTimestamps;
  bool skipNotSymbolicObjects;
  bool skipNotLazyInitialized;
  bool skipLocal;
  bool skipGlobal;
  unsigned timestamp;
  ExecutionState &state;
  KType *objectType;
  bool complete;
  bool allowAll = false;

public:
  explicit ResolvePredicate(ExecutionState &state, ref<PointerExpr> address,
                            KType *objectType, bool complete)
      : useTimestamps(UseTimestamps),
        skipNotSymbolicObjects(SkipNotSymbolicObjects),
        skipNotLazyInitialized(SkipNotLazyInitialized), skipLocal(SkipLocal),
        skipGlobal(SkipGlobal), timestamp(UINT_MAX), state(state),
        objectType(objectType), complete(complete) {
    ref<Expr> base = address->getBase();
    // TODO: skip
    if (isa<ConstantExpr>(base)) {
      allowAll = true;
      return;
    }
    // if (!isa<ConstantExpr>(base)) {
    //   std::pair<ref<const MemoryObject>, ref<Expr>> moBasePair;
    //   if (state.getBase(base, moBasePair)) {
    //     timestamp = moBasePair.first->timestamp;
    //   }
    // }

    // This is hack to deal with the poineters, which are represented as
    // select expressions from constant pointers with symbolic condition.
    // in such case we allow to resolve in all objects in the address space,
    // but should forbid lazy initilization.
    if (isa<SelectExpr>(base)) {
      auto alternatives =
          ArrayExprHelper::collectAlternatives(cast<SelectExpr>(*base));

      if (std::find_if_not(alternatives.begin(), alternatives.end(),
                           [](ref<Expr> expr) {
                             return isa<ConstantExpr>(expr);
                           }) == alternatives.end()) {
        skipNotSymbolicObjects = false;
        skipNotLazyInitialized = false;
        skipLocal = false;
        skipGlobal = false;
      }
    }
  }

  bool operator()(const MemoryObject *mo, const ObjectState *os) const {
    if (allowAll) {
      return true;
    }

    bool result = !useTimestamps || mo->timestamp <= timestamp;
    result = result && (!skipNotSymbolicObjects || state.inSymbolics(*mo));
    result = result && (!skipNotLazyInitialized || mo->isLazyInitialized);
    result = result && (!skipLocal || !mo->isLocal);
    result = result && (!skipGlobal || !mo->isGlobal);
    result = result && os->isAccessableFrom(objectType);
    result = result && (!complete || os->wasWritten);
    return result;
  }
};

ResolveResult<ResolutionList> AddressSpace::resolveSymbolic(
    ExecutionState &state, TimingSolver *solver, ref<PointerExpr> p,
    KType *objectType, unsigned maxResolutions, time::Span timeout) const {
  ResolvePredicate predicate(state, p, objectType, complete);
  TimerStatIncrementer timer(stats::resolveTime);

  ResolutionList rl;

  for (MemoryMap::iterator oi = objects.begin(), oe = objects.end(); oi != oe;
       ++oi) {
    const MemoryObject *mo = oi->first;
    if (!predicate(mo, oi->second.get())) {
      continue;
    }

    if (timeout && timeout < timer.delta()) {
      return ResolveResult<ResolutionList>::createUnknown(rl);
    }

    auto op = std::make_pair<>(mo, oi->second.get());

    int incomplete =
        checkPointerInObject(state, solver, p, op, rl, maxResolutions);
    if (incomplete != 2) {
      return incomplete ? ResolveResult<ResolutionList>::createUnknown(rl)
                        : ResolveResult<ResolutionList>::createOk(rl);
    }
  }

  return ResolveResult<ResolutionList>::createOk(rl);
}

ResolveResult<ObjectPair>
AddressSpace::resolveOneSymbolic(ExecutionState &state, TimingSolver *solver,
                                 ref<PointerExpr> address, KType *objectType,
                                 const std::atomic_bool &haltExecution) const {
  ResolvePredicate predicate(state, address, objectType, complete);

  for (MemoryMap::iterator oi = objects.begin(), oe = objects.end(); oi != oe;
       ++oi) {
    ObjectPair objectPair(oi->first, oi->second.get());
    const auto &mo = oi->first;
    if (!predicate(mo, oi->second.get())) {
      continue;
    }

    if (haltExecution) {
      break;
    }

    bool mayBeTrue;
    if (!solver->mayBeTrue(state.constraints.cs(),
                           mo->getBoundsCheckPointer(address), mayBeTrue,
                           state.queryMetaData)) {
      return ResolveResult<ObjectPair>::createUnknown(objectPair);
    }

    if (mayBeTrue) {
      return ResolveResult<ObjectPair>::createOk(objectPair);
    }
  }

  return ResolveResult<ObjectPair>::createNone();
}

ResolveResult<ObjectPair>
AddressSpace::resolveOne(ExecutionState &state, TimingSolver *solver,
                         ref<PointerExpr> address, KType *objectType,
                         const std::atomic_bool &haltExecution) const {
  if (ref<ConstantPointerExpr> CP = dyn_cast<ConstantPointerExpr>(address)) {
    if (auto resolveResult = resolveOne(CP, objectType)) {
      return resolveResult;
    }
  }

  TimerStatIncrementer timer(stats::resolveTime);

  // try cheap search, will succeed for any inbounds pointer

  ref<ConstantPointerExpr> addressCex;
  if (!solver->getValue(state.constraints.cs(), address, addressCex,
                        state.queryMetaData)) {
    return ResolveResult<ObjectPair>::createUnknown({});
  }

  if (auto resolveResult = resolveOne(addressCex, objectType)) {
    return resolveResult;
  }

  if (address->areAliasedBasesKnown()) {
    for (auto base : address->getAliasedBases()) {
      auto constantBase = cast<ConstantExpr>(base);
      auto constantBaseValue = constantBase->getZExtValue();

      if (idsToObjects.count(constantBaseValue) == 0) {
        continue;
      }
      auto mo = idsToObjects.at(constantBaseValue);

      auto resolution = findObject(mo);
      if (resolution.isNone()) {
        continue;
      }
      assert(resolution.isOk());

      if (haltExecution) {
        break;
      }

      bool mayBeTrue;
      if (!solver->mayBeTrue(state.constraints.cs(),
                             mo->getBoundsCheckPointer(address), mayBeTrue,
                             state.queryMetaData)) {
        return ResolveResult<ObjectPair>::createUnknown({});
      }

      if (mayBeTrue) {
        return resolution;
      }
    }
    return ResolveResult<ObjectPair>::createNone();
  } else {
    return resolveOneSymbolic(state, solver, address, objectType,
                              haltExecution);
  }
}

int AddressSpace::checkPointerInObject(ExecutionState &state,
                                       TimingSolver *solver, ref<PointerExpr> p,
                                       const ObjectPair &op, ResolutionList &rl,
                                       unsigned maxResolutions) const {
  // XXX in the common case we can save one query if we ask
  // mustBeTrue before mayBeTrue for the first result. easy
  // to add I just want to have a nice symbolic test case first.
  const MemoryObject *mo = op.first;
  ref<Expr> inBounds = mo->getBoundsCheckPointer(p);

  bool mayBeTrue;
  if (!solver->mayBeTrue(state.constraints.cs(), inBounds, mayBeTrue,
                         state.queryMetaData)) {
    return 1;
  }

  if (mayBeTrue) {
    rl.push_back(op);

    // fast path check
    auto size = rl.size();
    if (size == 1) {
      bool mustBeTrue;
      if (!solver->mustBeTrue(state.constraints.cs(), inBounds, mustBeTrue,
                              state.queryMetaData))
        return 1;
      if (mustBeTrue)
        return 0;
    } else if (size == maxResolutions)
      return 1;
  }

  return 2;
}

ResolveResult<ResolutionList>
AddressSpace::resolve(ExecutionState &state, TimingSolver *solver,
                      ref<PointerExpr> p, KType *objectType,
                      unsigned maxResolutions, time::Span timeout) const {
  ResolutionList rl;

  if (ref<ConstantPointerExpr> CP = dyn_cast<ConstantPointerExpr>(p)) {
    if (auto resolveResult = resolveOne(CP, objectType)) {
      rl.push_back(resolveResult.get());
      return ResolveResult<ResolutionList>::createOk(rl);
    }
  }
  TimerStatIncrementer timer(stats::resolveTime);

  ////////////////////////////////////////////////////////////////

  // XXX in general this isn't exactly what we want... for
  // a multiple resolution case (or for example, a \in {b,c,0})
  // we want to find the first object, find a cex assuming
  // not the first, find a cex assuming not the second...
  // etc.

  // XXX how do we smartly amortize the cost of checking to
  // see if we need to keep searching up/down, in bad cases?
  // maybe we don't care?

  // XXX we really just need a smart place to start (although
  // if its a known solution then the code below is guaranteed
  // to hit the fast path with exactly 2 queries). we could also
  // just get this by inspection of the expr.

  if (p->areAliasedBasesKnown()) {
    for (auto base : p->getAliasedBases()) {
      auto constantBase = cast<ConstantExpr>(base);
      auto constantBaseValue = constantBase->getZExtValue();

      if (idsToObjects.count(constantBaseValue) == 0) {
        continue;
      }
      auto memoryObject = idsToObjects.at(constantBaseValue);

      auto resolution = findObject(memoryObject);
      if (resolution.isNone()) {
        continue;
      }
      assert(resolution.isOk());
      auto objectPair = resolution.get();

      if (timeout && timeout < timer.delta()) {
        return ResolveResult<ResolutionList>::createUnknown(rl);
      }

      int incomplete = checkPointerInObject(state, solver, p, objectPair, rl,
                                            maxResolutions);
      if (incomplete != 2) {
        return incomplete ? ResolveResult<ResolutionList>::createUnknown(rl)
                          : ResolveResult<ResolutionList>::createOk(rl);
      }
    }
    return ResolveResult<ResolutionList>::createOk(rl);
  } else {
    return resolveSymbolic(state, solver, p, objectType);
  }
}

// These two are pretty big hack so we can sort of pass memory back
// and forth to externals. They work by abusing the concrete cache
// store inside of the object states, which allows them to
// transparently avoid screwing up symbolics (if the byte is symbolic
// then its concrete cache byte isn't being used) but is just a hack.

void AddressSpace::copyOutConcretes(const Assignment &assignment) {
  for (const auto &object : objects) {
    auto &mo = object.first;
    auto &os = object.second;
    if (ref<ConstantExpr> sizeExpr =
            dyn_cast<ConstantExpr>(mo->getSizeExpr())) {
      if (!mo->isUserSpecified && !os->readOnly &&
          sizeExpr->getZExtValue() != 0) {
        copyOutConcrete(mo, os.get(), assignment);
      }
    }
  }
}

ref<ConstantExpr> toConstantExpr(ref<Expr> expr) {
  if (ref<ConstantPointerExpr> pointer = dyn_cast<ConstantPointerExpr>(expr)) {
    return pointer->getConstantValue();
  } else {
    return cast<ConstantExpr>(expr);
  }
}

void AddressSpace::copyOutConcrete(const MemoryObject *mo,
                                   const ObjectState *os,
                                   const Assignment &assignment) const {
  if (ref<ConstantPointerExpr> addressExpr =
          dyn_cast<ConstantPointerExpr>(mo->getBaseExpr())) {
    auto address = reinterpret_cast<std::uint8_t *>(
        addressExpr->getConstantValue()->getZExtValue());
    AssignmentEvaluator evaluator(assignment, false);
    if (ref<ConstantExpr> sizeExpr =
            dyn_cast<ConstantExpr>(mo->getSizeExpr())) {
      size_t moSize = sizeExpr->getZExtValue();
      std::vector<uint8_t> concreteStore(moSize);
      for (size_t i = 0; i < moSize; i++) {
        auto byte = evaluator.visit(os->readValue8(i));
        concreteStore[i] = cast<ConstantExpr>(byte)->getZExtValue(Expr::Int8);
      }
      std::memcpy(address, concreteStore.data(), moSize);
    }
  }
}

bool AddressSpace::copyInConcretes(ExecutionState &state,
                                   const Assignment &assignment) {
  for (auto &obj : objects) {
    const MemoryObject *mo = obj.first;

    if (!mo->isUserSpecified) {
      const auto &os = obj.second;
      if (ref<ConstantPointerExpr> arrayConstantAddress =
              dyn_cast<ConstantPointerExpr>(mo->getBaseExpr())) {
        if (!copyInConcrete(
                state, mo, os.get(),
                arrayConstantAddress->getConstantValue()->getZExtValue(),
                assignment))
          return false;
      }
    }
  }

  return true;
}

bool AddressSpace::copyInConcrete(ExecutionState &state, const MemoryObject *mo,
                                  const ObjectState *os, uint64_t src_address,
                                  const Assignment &assignment) {
  AssignmentEvaluator evaluator(assignment, false);
  auto address = reinterpret_cast<std::uint8_t *>(src_address);
  size_t moSize =
      cast<ConstantExpr>(evaluator.visit(mo->getSizeExpr()))->getZExtValue();
  std::vector<uint8_t> concreteStore(moSize);
  for (size_t i = 0; i < moSize; i++) {
    auto byte = evaluator.visit(os->readValue8(i));
    concreteStore[i] = cast<ConstantExpr>(byte)->getZExtValue(8);
  }
  if (memcmp(address, concreteStore.data(), moSize) != 0) {
    if (os->readOnly) {
      return false;
    } else {
      ObjectState *wos = getWriteable(mo, os);
      if (state.inSymbolics(*mo)) {
        state.replaceSymbolic(*mo, *os, *wos);
      }
      for (size_t i = 0; i < moSize; i++) {
        wos->write(i, ConstantExpr::create(address[i], Expr::Int8));
      }
    }
  }
  return true;
}

/***/

bool MemoryObjectLT::operator()(const MemoryObject *a,
                                const MemoryObject *b) const {
  if (a->address.has_value() && b->address.has_value()) {
    return a->address < b->address;
  } else if (a->address.has_value()) {
    return true;
  } else if (b->address.has_value()) {
    return false;
  }
  return a->getBaseExpr() < b->getBaseExpr();
}
