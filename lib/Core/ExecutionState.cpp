//===-- ExecutionState.cpp ------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ExecutionState.h"

#include "ConstructStorage.h"
#include "Memory.h"

#include "klee/Expr/Expr.h"
#include "klee/Module/Cell.h"
#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"
#include "klee/Support/Casting.h"
#include "klee/Support/OptionCategories.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iomanip>
#include <klee/Support/CompilerWarning.h>
#include <set>
#include <sstream>
#include <string>

using namespace llvm;
using namespace klee;

namespace klee {
cl::opt<unsigned long long> MaxCyclesBeforeStuck(
    "max-cycles-before-stuck",
    cl::desc("Set target after after state visiting some basic block this "
             "amount of times (default=1)."),
    cl::init(1), cl::cat(TerminationCat));
}

namespace {
cl::opt<bool> UseGEPOptimization(
    "use-gep-opt", cl::init(true),
    cl::desc("Lazily initialize whole objects referenced by gep expressions "
             "instead of only the referenced parts (default=true)"),
    cl::cat(ExecCat));

} // namespace

/***/

std::uint32_t ExecutionState::nextID = 1;

/***/

void ExecutionStack::pushFrame(KInstIterator caller, KFunction *kf) {
  valueStack_.emplace_back(StackFrame(kf));
  if (std::find(callStack_.begin(), callStack_.end(),
                CallStackFrame(caller, kf)) == callStack_.end()) {
    uniqueFrames_.emplace_back(CallStackFrame(caller, kf));
  }
  callStack_.emplace_back(CallStackFrame(caller, kf));
  infoStack_.emplace_back(InfoStackFrame(kf));
  auto kfLevel = multilevel[kf].second;
  multilevel.replace({kf, kfLevel + 1});
  ++stackBalance;
  assert(valueStack_.size() == callStack_.size());
  assert(valueStack_.size() == infoStack_.size());
  stackSize += kf->getNumRegisters();
}

void ExecutionStack::popFrame() {
  assert(callStack_.size() > 0);
  KInstIterator caller = callStack_.back().caller;
  KFunction *kf = callStack_.back().kf;
  valueStack_.pop_back();
  callStack_.pop_back();
  infoStack_.pop_back();
  auto it = std::find(callStack_.begin(), callStack_.end(),
                      CallStackFrame(caller, kf));
  if (it == callStack_.end()) {
    uniqueFrames_.pop_back();
  }
  auto kfLevel = multilevel[kf].second;
  multilevel.replace({kf, kfLevel - 1});
  --stackBalance;
  assert(valueStack_.size() == callStack_.size());
  assert(valueStack_.size() == infoStack_.size());
  stackSize -= kf->getNumRegisters();
}

bool CallStackFrame::equals(const CallStackFrame &other) const {
  return kf == other.kf && caller == other.caller;
}

StackFrame::StackFrame(KFunction *kf) : kf(kf), varargs(nullptr) {
  locals.reset(constructStorage<Cell>(kf->getNumRegisters()));
}

StackFrame::StackFrame(const StackFrame &s)
    : kf(s.kf), allocas(s.allocas), varargs(s.varargs) {
  locals.reset(s.locals->clone());
}

StackFrame::~StackFrame() {}

InfoStackFrame::InfoStackFrame(KFunction *kf) : kf(kf) {}

/***/
ExecutionState::ExecutionState()
    : initPC(nullptr), pc(nullptr), prevPC(nullptr), incomingBBIndex(-1),
      depth(0), ptreeNode(nullptr), steppedInstructions(0),
      steppedMemoryInstructions(0), instsSinceCovNew(0),
      roundingMode(llvm::APFloat::rmNearestTiesToEven), coveredNew({}),
      coveredNewError(new box<bool>(false)), forkDisabled(false),
      prevHistory_(TargetsHistory::create()),
      history_(TargetsHistory::create()) {
  setID();
}

ExecutionState::ExecutionState(KFunction *kf)
    : initPC(kf->instructions), pc(initPC), prevPC(pc), incomingBBIndex(-1),
      depth(0), ptreeNode(nullptr), steppedInstructions(0),
      steppedMemoryInstructions(0), instsSinceCovNew(0),
      roundingMode(llvm::APFloat::rmNearestTiesToEven), coveredNew({}),
      coveredNewError(new box<bool>(false)), forkDisabled(false),
      prevHistory_(TargetsHistory::create()),
      history_(TargetsHistory::create()) {
  pushFrame(nullptr, kf);
  setID();
}

ExecutionState::ExecutionState(KFunction *kf, KBlock *kb)
    : initPC(kb->instructions), pc(initPC), prevPC(pc), incomingBBIndex(-1),
      depth(0), ptreeNode(nullptr), steppedInstructions(0),
      steppedMemoryInstructions(0), instsSinceCovNew(0),
      roundingMode(llvm::APFloat::rmNearestTiesToEven), coveredNew({}),
      coveredNewError(new box<bool>(false)), forkDisabled(false),
      prevHistory_(TargetsHistory::create()),
      history_(TargetsHistory::create()) {
  pushFrame(nullptr, kf);
  setID();
}

ExecutionState::~ExecutionState() {
  while (!stack.empty())
    popFrame();
}

ExecutionState::ExecutionState(const ExecutionState &state)
    : initPC(state.initPC), pc(state.pc), prevPC(state.prevPC),
      stack(state.stack), stackBalance(state.stackBalance),
      incomingBBIndex(state.incomingBBIndex), depth(state.depth),
      level(state.level), addressSpace(state.addressSpace),
      constraints(state.constraints), eventsRecorder(state.eventsRecorder),
      targetForest(state.targetForest), pathOS(state.pathOS),
      symPathOS(state.symPathOS), coveredLines(state.coveredLines),
      symbolics(state.symbolics), stateSymbolicsNum(state.stateSymbolicsNum),
      resolvedPointers(state.resolvedPointers),
      cexPreferences(state.cexPreferences), arrayNames(state.arrayNames),
      steppedInstructions(state.steppedInstructions),
      steppedMemoryInstructions(state.steppedMemoryInstructions),
      instsSinceCovNew(state.instsSinceCovNew),
      roundingMode(state.roundingMode),
      unwindingInformation(state.unwindingInformation
                               ? state.unwindingInformation->clone()
                               : nullptr),
      coveredNew(state.coveredNew), coveredNewError(state.coveredNewError),
      forkDisabled(state.forkDisabled), returnValue(state.returnValue),
      gepExprBases(state.gepExprBases), multiplexKF(state.multiplexKF),
      prevTargets_(state.prevTargets_), targets_(state.targets_),
      prevHistory_(state.prevHistory_), history_(state.history_),
      isTargeted_(state.isTargeted_) {
  queryMetaData.id = state.id;
}

ExecutionState *ExecutionState::branch() {
  depth++;

  auto *falseState = new ExecutionState(*this);
  falseState->setID();
  falseState->coveredLines.clear();
  falseState->prevTargets_ = falseState->targets_;
  falseState->prevHistory_ = falseState->history_;

  return falseState;
}

ExecutionState *ExecutionState::withStackFrame(KInstIterator caller,
                                               KFunction *kf) {
  ExecutionState *newState = new ExecutionState(*this);
  newState->setID();
  newState->pushFrame(caller, kf);
  newState->initPC = kf->blockMap[&*kf->function()->begin()]->instructions;
  newState->pc = newState->initPC;
  newState->prevPC = newState->pc;
  return newState;
}

ExecutionState *ExecutionState::withKInstruction(KInstruction *ki) const {
  assert(stack.size() == 0);
  ExecutionState *newState = new ExecutionState(*this);
  newState->setID();
  newState->pushFrame(nullptr, ki->parent->parent);
  newState->stackBalance = 0;
  newState->initPC = ki->parent->instructions;
  while (newState->initPC != ki) {
    ++newState->initPC;
  }
  newState->pc = newState->initPC;
  newState->prevPC = newState->pc;
  return newState;
}

ExecutionState *ExecutionState::withKFunction(KFunction *kf) {
  return withStackFrame(nullptr, kf);
}

ExecutionState *ExecutionState::copy() const {
  ExecutionState *newState = new ExecutionState(*this);
  newState->setID();
  return newState;
}

void ExecutionState::pushFrame(KInstIterator caller, KFunction *kf) {
  stack.pushFrame(caller, kf);
}

void ExecutionState::popFrame() {
  const StackFrame &sf = stack.valueStack().back();
  for (auto &memoryObject : sf.allocas) {
    assert(memoryObject);
    removePointerResolutions(memoryObject.get());
    addressSpace.unbindObject(memoryObject.get());
  }
  stack.popFrame();
}

void ExecutionState::addSymbolic(const MemoryObject &object,
                                 const ObjectState &state) {
  if (symbolics.count(&object) == 0) {
    symbolics = symbolics.insert(
        {&object, ImmutableSet{Symbolic(&object, &state, stateSymbolicsNum)}});
  } else {
    const auto &oldSymbolicsOfObject = symbolics.at(&object);
    auto newSymbolicsOfObject = oldSymbolicsOfObject.insert(
        Symbolic(&object, &state, stateSymbolicsNum));
    symbolics = symbolics.replace({&object, newSymbolicsOfObject});
  }
  ++stateSymbolicsNum;
}

void ExecutionState::replaceSymbolic(const MemoryObject &object,
                                     const ObjectState &oldState,
                                     const ObjectState &newState) {
  assert(symbolics.count(&object) != 0);
  const auto &oldSymbolicsSet = symbolics.at(&object);

  // FIXME: search to log(n)
  for (const auto &symbolic : oldSymbolicsSet) {
    if (symbolic.objectState.get() == &oldState) {
      Symbolic newSymbolic(&object, &newState, symbolic.num);
      auto newSymbolicSet =
          oldSymbolicsSet.remove(symbolic).insert(newSymbolic);
      symbolics = symbolics.replace({&object, newSymbolicSet});
      return;
    }
  }
  assert(false);
  unreachable();
}

ref<const MemoryObject>
ExecutionState::findMemoryObject(const Array *array) const {
  // FIXME: use hash map instead
  for (auto &[mo, symbolicsSet] : symbolics) {
    for (auto symbolic : symbolicsSet) {
      if (array == symbolic.array()) {
        return mo;
      }
    }
  }
  return nullptr;
}

void ExecutionState::replaceMemoryObjectFromSymbolics(
    const MemoryObject &oldObject, const MemoryObject &newObject,
    const ObjectState &oldState, const ObjectState &newState) {
  assert(symbolics.count(&oldObject) != 0);

  assert(&oldObject != &newObject);

  std::optional<std::size_t> symbolicNum;
  {
    const auto &oldSymbolicsSet = symbolics.at(&oldObject);

    // FIXME: search to log(n)
    for (const auto &symbolic : oldSymbolicsSet) {
      if (symbolic.objectState.get() == &oldState) {
        symbolicNum.emplace(symbolic.num);
        symbolics =
            symbolics.replace({&oldObject, oldSymbolicsSet.remove(symbolic)});
        break;
      }
    }
  }

  if (symbolics.count(&newObject) == 0) {
    symbolics = symbolics.insert(
        {&newObject,
         ImmutableSet{Symbolic(&newObject, &newState, *symbolicNum)}});
  } else {
    const auto &oldSymbolicsOfObject = symbolics.at(&newObject);
    auto newSymbolicsOfObject = oldSymbolicsOfObject.insert(
        Symbolic(&newObject, &newState, *symbolicNum));
    symbolics = symbolics.replace({&newObject, newSymbolicsOfObject});
  }
}

bool ExecutionState::inSymbolics(const MemoryObject &mo) const {
  return symbolics.count(&mo) > 0;
}

bool ExecutionState::getBase(
    ref<Expr> expr,
    std::pair<ref<const MemoryObject>, ref<Expr>> &resolution) const {
  ref<ReadExpr> base = expr->hasOrderedReads();
  if (!base) {
    return false;
  }
  if (ref<LazyInitializationAddressSource> liaSource =
          dyn_cast<LazyInitializationAddressSource>(
              base->updates.root->source)) {
    return getBase(liaSource->pointer, resolution);
  }
  auto parent = findMemoryObject(base->updates.root);
  if (!parent) {
    return false;
  }
  resolution = std::make_pair(parent, base->index);
  return true;
}

void ExecutionState::removePointerResolutions(const MemoryObject *mo) {
  for (auto resolution = begin(resolvedPointers);
       resolution != end(resolvedPointers);) {
    resolution->second.erase(mo);
    if (resolution->second.size() == 0) {
      resolution = resolvedPointers.erase(resolution);
    } else {
      ++resolution;
    }
  }

  for (auto resolution = begin(resolvedSubobjects);
       resolution != end(resolvedSubobjects);) {
    resolution->second.erase(mo);
    if (resolution->second.size() == 0) {
      resolution = resolvedSubobjects.erase(resolution);
    } else {
      ++resolution;
    }
  }
}

void ExecutionState::removePointerResolutions(ref<PointerExpr> address,
                                              unsigned size) {
  ref<Expr> base = address->getBase();
  if (!isa<ConstantExpr>(base)) {
    resolvedPointers[base].clear();
    resolvedSubobjects[MemorySubobject(address, size)].clear();
  }
}

// base address mo and ignore non pure reads in setinitializationgraph
void ExecutionState::addPointerResolution(ref<PointerExpr> address,
                                          const MemoryObject *mo,
                                          unsigned size) {
  ref<Expr> base = address->getBase();
  if (!isa<ConstantExpr>(base)) {
    resolvedPointers[base].insert(mo);
    resolvedSubobjects[MemorySubobject(address, size)].insert(mo);
  }
}

void ExecutionState::addUniquePointerResolution(ref<PointerExpr> address,
                                                const MemoryObject *mo,
                                                unsigned size) {
  ref<Expr> base = address->getBase();
  if (!isa<ConstantExpr>(base)) {
    removePointerResolutions(address, size);
    resolvedPointers[base].insert(mo);
    resolvedSubobjects[MemorySubobject(address, size)].insert(mo);
  }
}

/**/

llvm::raw_ostream &klee::operator<<(llvm::raw_ostream &os,
                                    const MemoryMap &mm) {
  os << "{";
  MemoryMap::iterator it = mm.begin();
  MemoryMap::iterator ie = mm.end();
  if (it != ie) {
    os << "MO" << it->first->id << ":" << it->second.get();
    for (++it; it != ie; ++it)
      os << ", MO" << it->first->id << ":" << it->second.get();
  }
  os << "}";
  return os;
}

void ExecutionState::dumpStack(llvm::raw_ostream &out) const {
  const KInstruction *target = prevPC;
  for (unsigned i = 0; i < stack.size(); ++i) {
    unsigned ri = stack.size() - 1 - i;
    const CallStackFrame &csf = stack.callStack().at(ri);
    const StackFrame &sf = stack.valueStack().at(ri);

    Function *f = csf.kf->function();
    out << "\t#" << i;
    auto assemblyLine = target->getKModule()->getAsmLine(target->inst());
    if (assemblyLine.has_value()) {
      std::stringstream AsmStream;
      AsmStream << std::setw(8) << std::setfill('0') << assemblyLine.value();
      out << AsmStream.str();
    }
    out << " in " << f->getName().str() << "(";
    // Yawn, we could go up and print varargs if we wanted to.
    unsigned index = 0;
    for (Function::arg_iterator ai = f->arg_begin(), ae = f->arg_end();
         ai != ae; ++ai) {
      if (ai != f->arg_begin())
        out << ", ";

      if (ai->hasName())
        out << ai->getName().str() << "=";

      ref<Expr> value = sf.locals->at(csf.kf->getArgRegister(index++)).value;
      if (isa_and_nonnull<ConstantExpr>(value)) {
        out << value;
      } else if (isa_and_nonnull<ConstantPointerExpr>(value)) {
        out << value;
      } else {
        out << "symbolic";
      }
    }
    out << ")";
    std::string filepath = target->getSourceFilepath();
    if (!filepath.empty())
      out << " at " << filepath << ":" << target->getLine();
    out << "\n";
    target = csf.caller;
  }
}

void ExecutionState::addConstraint(ref<Expr> e) {
  constraints.addConstraint(e);
}

void ExecutionState::addCexPreference(const ref<Expr> &cond) {
  cexPreferences = cexPreferences.insert(cond);
}

BasicBlock *ExecutionState::getInitPCBlock() const {
  return initPC->inst()->getParent();
}

BasicBlock *ExecutionState::getPrevPCBlock() const {
  return prevPC->inst()->getParent();
}

BasicBlock *ExecutionState::getPCBlock() const {
  return pc->inst()->getParent();
}

void ExecutionState::increaseLevel() {
  llvm::BasicBlock *srcbb = getPrevPCBlock();
  KFunction *kf = prevPC->parent->parent;
  KModule *kmodule = kf->parent;

  if (prevPC->inst()->isTerminator() &&
      kmodule->inMainModule(*kf->function())) {
    auto srcLevel = stack.infoStack().back().multilevel[srcbb].second;
    stack.infoStack().back().multilevel.replace({srcbb, srcLevel + 1});
    if (afterFork) {
      auto symSrcLevel =
          stack.infoStack().back().symbolicMultilevel[srcbb].second;
      stack.infoStack().back().symbolicMultilevel.replace(
          {srcbb, symSrcLevel + 1});
    }
    level.insert(prevPC->parent);
  }
}

bool ExecutionState::isGEPExpr(ref<Expr> expr) const {
  return UseGEPOptimization && gepExprBases.find(expr) != gepExprBases.end();
}

bool ExecutionState::visited(KBlock *block) const {
  return level.count(block) != 0;
}

bool ExecutionState::reachedTarget(ref<ReachBlockTarget> target) const {
  if (constraints.path().KBlockSize() == 0) {
    return false;
  }
  if (target->isAtEnd()) {
    return prevPC == target->getBlock()->getLastInstruction();
  } else {
    return pc == target->getBlock()->getFirstInstruction();
  }
}

Query ExecutionState::toQuery(ref<Expr> head) const {
  return Query(constraints.cs(), head, id);
}

Query ExecutionState::toQuery() const { return toQuery(Expr::createFalse()); }
