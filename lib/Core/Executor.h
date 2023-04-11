//===-- Executor.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Class to perform actual execution, hides implementation details from external
// interpreter.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_EXECUTOR_H
#define KLEE_EXECUTOR_H

#include "ExecutionState.h"
#include "UserSearcher.h"

#include "klee/ADT/RNG.h"
#include "klee/Core/BranchTypes.h"
#include "klee/Core/Interpreter.h"
#include "klee/Core/TerminationTypes.h"
#include "klee/Expr/ArrayCache.h"
#include "klee/Expr/ArrayExprOptimizer.h"
#include "klee/Expr/SourceBuilder.h"
#include "klee/Expr/SymbolicSource.h"
#include "klee/Module/Cell.h"
#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"
#include "klee/Solver/ConcretizationManager.h"
#include "klee/System/Time.h"

#include "llvm/ADT/Twine.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct KTest;

namespace llvm {
class BasicBlock;
class BranchInst;
class CallInst;
class LandingPadInst;
class Constant;
class ConstantExpr;
class Function;
class GlobalValue;
class Instruction;
class LLVMContext;
class DataLayout;
class Twine;
class Value;
} // namespace llvm

namespace klee {
class AddressManager;
class Array;
struct Cell;
class CodeGraphDistance;
class ExecutionState;
class ExternalDispatcher;
class Expr;
template <class T> class ExprHashMap;
class InstructionInfoTable;
class KCallable;
struct KFunction;
struct KInstruction;
class KInstIterator;
class KModule;
class MemoryManager;
class MemoryObject;
class ObjectState;
class PForest;
class Searcher;
class SeedInfo;
class SpecialFunctionHandler;
struct StackFrame;
class SymbolicSource;
class TargetCalculator;
class StatsTracker;
class TimingSolver;
class TreeStreamWriter;
class TypeManager;
class MergeHandler;
class MergingSearcher;
template <class T> class ref;

/// \todo Add a context object to keep track of data only live
/// during an instruction step. Should contain addedStates,
/// removedStates, and haltExecution, among others.

class Executor : public Interpreter {
  friend class OwningSearcher;
  friend class WeightedRandomSearcher;
  friend class SpecialFunctionHandler;
  friend class StatsTracker;
  friend class MergeHandler;
  friend klee::Searcher *klee::constructUserSearcher(Executor &executor);

public:
  typedef std::pair<ExecutionState *, ExecutionState *> StatePair;

  /// The random number generator.
  RNG theRNG;

private:
  using SetOfStates = std::set<ExecutionState *, ExecutionStateIDCompare>;
  /* Set of Intrinsic::ID. Plain type is used here to avoid including llvm in
   * the header */
  static const std::unordered_set<llvm::Intrinsic::ID> supportedFPIntrinsics;
  static const std::unordered_set<llvm::Intrinsic::ID> modelledFPIntrinsics;

  std::unique_ptr<KModule> kmodule;
  InterpreterHandler *interpreterHandler;
  Searcher *searcher;

  ExternalDispatcher *externalDispatcher;
  TimingSolver *solver;
  std::unique_ptr<AddressManager> addressManager;
  MemoryManager *memory;
  TypeManager *typeSystemManager;

  SetOfStates states;
  SetOfStates pausedStates;
  StatsTracker *statsTracker;
  TreeStreamWriter *pathWriter, *symPathWriter;
  SpecialFunctionHandler *specialFunctionHandler;
  TimerGroup timers;
  std::unique_ptr<ConcretizationManager> concretizationManager;
  std::unique_ptr<PForest> processForest;
  std::unique_ptr<CodeGraphDistance> codeGraphDistance;
  std::unique_ptr<TargetCalculator> targetCalculator;

  /// Used to track states that have been added during the current
  /// instructions step.
  /// \invariant \ref addedStates is a subset of \ref states.
  /// \invariant \ref addedStates and \ref removedStates are disjoint.
  std::vector<ExecutionState *> addedStates;
  /// Used to track states that have been removed during the current
  /// instructions step.
  /// \invariant \ref removedStates is a subset of \ref states.
  /// \invariant \ref addedStates and \ref removedStates are disjoint.
  std::vector<ExecutionState *> removedStates;

  /// When non-empty the Executor is running in "seed" mode. The
  /// states in this map will be executed in an arbitrary order
  /// (outside the normal search interface) until they terminate. When
  /// the states reach a symbolic branch then either direction that
  /// satisfies one or more seeds will be added to this map. What
  /// happens with other states (that don't satisfy the seeds) depends
  /// on as-yet-to-be-determined flags.
  std::map<ExecutionState *, std::vector<SeedInfo>> seedMap;

  /// Map of globals to their representative memory object.
  std::map<const llvm::GlobalValue *, MemoryObject *> globalObjects;

  /// Map of globals to their bound address. This also includes
  /// globals that have no representative object (i.e. functions).
  std::map<const llvm::GlobalValue *, ref<ConstantExpr>> globalAddresses;

  /// Map of legal function addresses to the corresponding Function.
  /// Used to validate and dereference function pointers.
  std::unordered_map<std::uint64_t, llvm::Function *> legalFunctions;

  /// When non-null the bindings that will be used for calls to
  /// klee_make_symbolic in order replay.
  const struct KTest *replayKTest;

  /// When non-null a list of branch decisions to be used for replay.
  const std::vector<bool> *replayPath;

  /// The index into the current \ref replayKTest or \ref replayPath
  /// object.
  unsigned replayPosition;

  /// When non-null a list of "seed" inputs which will be used to
  /// drive execution.
  const std::vector<struct KTest *> *usingSeeds;

  /// Disables forking, instead a random path is chosen. Enabled as
  /// needed to control memory usage. \see fork()
  bool atMemoryLimit;

  /// Disables forking, set by client. \see setInhibitForking()
  bool inhibitForking;

  /// Signals the executor to halt execution at the next instruction
  /// step.
  HaltExecution::Reason haltExecution = HaltExecution::NotHalt;

  /// Whether implied-value concretization is enabled. Currently
  /// false, it is buggy (it needs to validate its writes).
  bool ivcEnabled;

  /// The maximum time to allow for a single core solver query.
  /// (e.g. for a single STP query)
  time::Span coreSolverTimeout;

  /// Maximum time to allow for a single instruction.
  time::Span maxInstructionTime;

  /// Assumes ownership of the created array objects
  ArrayCache arrayCache;

  /// File to print executed instructions to
  std::unique_ptr<llvm::raw_ostream> debugInstFile;

  // @brief Buffer used by logBuffer
  std::string debugBufferString;

  // @brief buffer to store logs before flushing to file
  llvm::raw_string_ostream debugLogBuffer;

  /// Optimizes expressions
  ExprOptimizer optimizer;

  /// Points to the merging searcher of the searcher chain,
  /// `nullptr` if merging is disabled
  MergingSearcher *mergingSearcher = nullptr;

  /// Typeids used during exception handling
  std::vector<ref<Expr>> eh_typeids;

  GuidanceKind guidanceKind;

  /// Return the typeid corresponding to a certain `type_info`
  ref<ConstantExpr> getEhTypeidFor(ref<Expr> type_info);

  void addHistoryResult(ExecutionState &state);

  void executeInstruction(ExecutionState &state, KInstruction *ki);

  void targetedRun(ExecutionState &initialState, KBlock *target,
                   ExecutionState **resultState = nullptr);

  void seed(ExecutionState &initialState);
  void run(std::vector<ExecutionState *> initialStates);
  void runWithTarget(ExecutionState &state, KFunction *kf, KBlock *target);

  void initializeTypeManager();

  // Given a concrete object in our [klee's] address space, add it to
  // objects checked code can reference.
  MemoryObject *addExternalObject(ExecutionState &state, void *addr, KType *,
                                  unsigned size, bool isReadOnly);

  void initializeGlobalAlias(const llvm::Constant *c, ExecutionState &state);
  void initializeGlobalObject(ExecutionState &state, ObjectState *os,
                              const llvm::Constant *c, unsigned offset);
  void initializeGlobals(ExecutionState &state);
  void allocateGlobalObjects(ExecutionState &state);
  void initializeGlobalAliases(ExecutionState &state);
  void initializeGlobalObjects(ExecutionState &state);

  void stepInstruction(ExecutionState &state);
  void updateStates(ExecutionState *current);
  void transferToBasicBlock(llvm::BasicBlock *dst, llvm::BasicBlock *src,
                            ExecutionState &state);
  void transferToBasicBlock(KBlock *dst, llvm::BasicBlock *src,
                            ExecutionState &state);

  void callExternalFunction(ExecutionState &state, KInstruction *target,
                            KCallable *callable,
                            std::vector<ref<Expr>> &arguments);

  ObjectState *bindObjectInState(ExecutionState &state, const MemoryObject *mo,
                                 KType *dynamicType, bool IsAlloca,
                                 const Array *array = 0);

  /// Resolve a pointer to the memory objects it could point to the
  /// start of, forking execution when necessary and generating errors
  /// for pointers to invalid locations (either out of bounds or
  /// address inside the middle of objects).
  ///
  /// \param results[out] A list of ((MemoryObject,ObjectState),
  /// state) pairs for each object the given address can point to the
  /// beginning of.
  typedef std::vector<std::pair<IDType, ExecutionState *>> ExactResolutionList;
  bool resolveExact(ExecutionState &state, ref<Expr> p, KType *type,
                    ExactResolutionList &results, const std::string &name);

  MemoryObject *allocate(ExecutionState &state, ref<Expr> size, bool isLocal,
                         bool isGlobal, const llvm::Value *allocSite,
                         size_t allocationAlignment,
                         ref<Expr> lazyInitializationSource = ref<Expr>(),
                         unsigned timestamp = 0);

  /// Allocate and bind a new object in a particular state. NOTE: This
  /// function may fork.
  ///
  /// \param isLocal Flag to indicate if the object should be
  /// automatically deallocated on function return (this also makes it
  /// illegal to free directly).
  ///
  /// \param target Value at which to bind the base address of the new
  /// object.
  ///
  /// \param reallocFrom If non-zero and the allocation succeeds,
  /// initialize the new object from the given one and unbind it when
  /// done (realloc semantics). The initialized bytes will be the
  /// minimum of the size of the old and new objects, with remaining
  /// bytes initialized as specified by zeroMemory.
  ///
  /// \param allocationAlignment If non-zero, the given alignment is
  /// used. Otherwise, the alignment is deduced via
  /// Executor::getAllocationAlignment
  void executeAlloc(ExecutionState &state, ref<Expr> size, bool isLocal,
                    KInstruction *target, KType *type, bool zeroMemory = false,
                    const ObjectState *reallocFrom = 0,
                    size_t allocationAlignment = 0,
                    bool checkOutOfMemory = false);

  /// Free the given address with checking for errors. If target is
  /// given it will be bound to 0 in the resulting states (this is a
  /// convenience for realloc). Note that this function can cause the
  /// state to fork and that \ref state cannot be safely accessed
  /// afterwards.
  void executeFree(ExecutionState &state, ref<Expr> address,
                   KInstruction *target = 0);

  /// Serialize a landingpad instruction so it can be handled by the
  /// libcxxabi-runtime
  MemoryObject *serializeLandingpad(ExecutionState &state,
                                    const llvm::LandingPadInst &lpi,
                                    bool &stateTerminated);

  /// Unwind the given state until it hits a landingpad. This is used
  /// for exception handling.
  void unwindToNextLandingpad(ExecutionState &state);

  void executeCall(ExecutionState &state, KInstruction *ki, llvm::Function *f,
                   std::vector<ref<Expr>> &arguments);

  // do address resolution / object binding / out of bounds checking
  // and perform the operation
  void executeMemoryOperation(ExecutionState &state, bool isWrite,
                              KType *targetType, ref<Expr> address,
                              ref<Expr> value /* undef if read */,
                              KInstruction *target /* undef if write */);

  IDType lazyInitializeObject(ExecutionState &state, ref<Expr> address,
                              KInstruction *target, KType *targetType,
                              uint64_t size);
  void executeMakeSymbolic(ExecutionState &state, const MemoryObject *mo,
                           KType *type, const std::string &name,
                           const ref<SymbolicSource> source, bool isLocal);
  void updateStateWithSymcretes(ExecutionState &state,
                                const Assignment &assignment);

  /// Create a new state where each input condition has been added as
  /// a constraint and return the results. The input state is included
  /// as one of the results. Note that the output vector may include
  /// NULL pointers for states which were unable to be created.
  void branch(ExecutionState &state, const std::vector<ref<Expr>> &conditions,
              std::vector<ExecutionState *> &result, BranchType reason);

  /// Fork current and return states in which condition holds / does
  /// not hold, respectively. One of the states is necessarily the
  /// current state, and one of the states may be null.
  StatePair fork(ExecutionState &current, ref<Expr> condition, bool isInternal,
                 BranchType reason);

  // If the MaxStatic*Pct limits have been reached, concretize the condition and
  // return it. Otherwise, return the unmodified condition.
  ref<Expr> maxStaticPctChecks(ExecutionState &current, ref<Expr> condition);

  /// Add the given (boolean) condition as a constraint on state. This
  /// function is a wrapper around the state's addConstraint function
  /// which also manages propagation of implied values,
  /// validity checks, and seed patching.
  void addConstraint(ExecutionState &state, ref<Expr> condition);

  // Called on [for now] concrete reads, replaces constant with a symbolic
  // Used for testing.
  ref<Expr> replaceReadWithSymbolic(ExecutionState &state, ref<Expr> e);

  ref<Expr> mockValue(ExecutionState &state, ref<Expr> result);

  const Cell &eval(KInstruction *ki, unsigned index, ExecutionState &state,
                   bool isSymbolic = true);

  Cell &getArgumentCell(ExecutionState &state, KFunction *kf, unsigned index) {
    return state.stack.back().locals[kf->getArgRegister(index)];
  }

  Cell &getDestCell(ExecutionState &state, KInstruction *target) {
    return state.stack.back().locals[target->dest];
  }

  void bindLocal(KInstruction *target, ExecutionState &state, ref<Expr> value);
  void bindArgument(KFunction *kf, unsigned index, ExecutionState &state,
                    ref<Expr> value);

  /// Evaluates an LLVM constant expression.  The optional argument ki
  /// is the instruction where this constant was encountered, or NULL
  /// if not applicable/unavailable.
  ref<klee::ConstantExpr> evalConstantExpr(const llvm::ConstantExpr *c,
                                           llvm::APFloat::roundingMode rm,
                                           const KInstruction *ki = NULL);

  /// Evaluates an LLVM float comparison. the operands are two float
  /// expressions.
  ref<klee::Expr> evaluateFCmp(unsigned int predicate, ref<klee::Expr> left,
                               ref<klee::Expr> right) const;

  /// Evaluates an LLVM constant.  The optional argument ki is the
  /// instruction where this constant was encountered, or NULL if
  /// not applicable/unavailable.
  ref<klee::ConstantExpr> evalConstant(const llvm::Constant *c,
                                       llvm::APFloat::roundingMode rm,
                                       const KInstruction *ki = NULL);

  /// Return a unique constant value for the given expression in the
  /// given state, if it has one (i.e. it provably only has a single
  /// value). Otherwise return the original expression.
  ref<Expr> toUnique(const ExecutionState &state, ref<Expr> &e);

  /// Return a constant value for the given expression, forcing it to
  /// be constant in the given state by adding a constraint if
  /// necessary. Note that this function breaks completeness and
  /// should generally be avoided.
  ///
  /// \param purpose An identify string to printed in case of concretization.
  ref<klee::ConstantExpr> toConstant(ExecutionState &state, ref<Expr> e,
                                     const char *purpose);

  /// Bind a constant value for e to the given target. NOTE: This
  /// function may fork state if the state has multiple seeds.
  void executeGetValue(ExecutionState &state, ref<Expr> e,
                       KInstruction *target);

  /// Get textual information regarding a memory address.
  std::string getAddressInfo(ExecutionState &state, ref<Expr> address,
                             const MemoryObject *mo = nullptr) const;

  // Determines the \param lastInstruction of the \param state which is not KLEE
  // internal and returns its InstructionInfo
  const InstructionInfo &
  getLastNonKleeInternalInstruction(const ExecutionState &state,
                                    llvm::Instruction **lastInstruction);

  /// Remove state from queue and delete state
  void terminateState(ExecutionState &state,
                      StateTerminationType terminationType);

  // pause state
  void pauseState(ExecutionState &state);

  // unpause state
  void unpauseState(ExecutionState &state);

  /// Call exit handler and terminate state normally
  /// (end of execution path)
  void terminateStateOnExit(ExecutionState &state);

  /// Call exit handler and terminate state early
  /// (e.g. due to state merging or memory pressure)
  void terminateStateEarly(ExecutionState &state, const llvm::Twine &message,
                           StateTerminationType terminationType);

  void reportStateOnTargetError(ExecutionState &state, ReachWithError error);

  /// Save extra information in targeted mode
  /// Then just call `terminateStateOnError`
  void terminateStateOnTargetError(ExecutionState &state, ReachWithError error);

  /// Call error handler and terminate state in case of program errors
  /// (e.g. free()ing globals, out-of-bound accesses)
  void terminateStateOnError(ExecutionState &state, const llvm::Twine &message,
                             StateTerminationType terminationType,
                             const llvm::Twine &longMessage = "",
                             const char *suffix = nullptr);

  void terminateStateOnTerminator(ExecutionState &state);

  /// Call error handler and terminate state in case of execution errors
  /// (things that should not be possible, like illegal instruction or
  /// unlowered intrinsic, or unsupported intrinsics, like inline assembly)
  void terminateStateOnExecError(ExecutionState &state,
                                 const llvm::Twine &message,
                                 const llvm::Twine &info = "");

  /// Call error handler and terminate state in case of solver errors
  /// (solver error or timeout)
  void terminateStateOnSolverError(ExecutionState &state,
                                   const llvm::Twine &message);

  /// Call error handler and terminate state for user errors
  /// (e.g. wrong usage of klee.h API)
  void terminateStateOnUserError(ExecutionState &state,
                                 const llvm::Twine &message);

  /// bindModuleConstants - Initialize the module constant table.
  void bindModuleConstants(const llvm::APFloat::roundingMode &rm);

  const Array *makeArray(ExecutionState &state, ref<Expr> size,
                         const std::string &name,
                         const ref<SymbolicSource> source);

  ExecutionState *prepareStateForPOSIX(KInstIterator &caller,
                                       ExecutionState *state);

  void prepareTargetedExecution(ExecutionState *initialState,
                                TargetForest whitelist);

  template <typename SqType, typename TypeIt>
  void computeOffsetsSeqTy(KGEPInstruction *kgepi,
                           ref<ConstantExpr> &constantOffset, uint64_t index,
                           const TypeIt it);

  template <typename TypeIt>
  void computeOffsets(KGEPInstruction *kgepi, TypeIt ib, TypeIt ie);

  /// bindInstructionConstants - Initialize any necessary per instruction
  /// constant values.
  void bindInstructionConstants(KInstruction *KI);

  void doImpliedValueConcretization(ExecutionState &state, ref<Expr> e,
                                    ref<ConstantExpr> value);

  /// check memory usage and terminate states when over threshold of -max-memory
  /// + 100MB \return true if below threshold, false otherwise (states were
  /// terminated)
  bool checkMemoryUsage();

  /// check if branching/forking is allowed
  bool branchingPermitted(const ExecutionState &state) const;

  void printDebugInstructions(ExecutionState &state);
  void doDumpStates();

  /// Only for debug purposes; enable via debugger or klee-control
  void dumpStates();
  void dumpPForest();

public:
  Executor(llvm::LLVMContext &ctx, const InterpreterOptions &opts,
           InterpreterHandler *ie);
  virtual ~Executor();

  const InterpreterHandler &getHandler() { return *interpreterHandler; }

  void setPathWriter(TreeStreamWriter *tsw) override { pathWriter = tsw; }

  void setSymbolicPathWriter(TreeStreamWriter *tsw) override {
    symPathWriter = tsw;
  }

  void setReplayKTest(const struct KTest *out) override {
    assert(!replayPath && "cannot replay both buffer and path");
    replayKTest = out;
    replayPosition = 0;
  }

  void setReplayPath(const std::vector<bool> *path) override {
    assert(!replayKTest && "cannot replay both buffer and path");
    replayPath = path;
    replayPosition = 0;
  }

  llvm::Module *
  setModule(std::vector<std::unique_ptr<llvm::Module>> &userModules,
            std::vector<std::unique_ptr<llvm::Module>> &libsModules,
            const ModuleOptions &opts,
            const std::vector<std::string> &mainModuleFunctions) override;

  void useSeeds(const std::vector<struct KTest *> *seeds) override {
    usingSeeds = seeds;
  }

  ExecutionState *formState(llvm::Function *f);
  ExecutionState *formState(llvm::Function *f, int argc, char **argv,
                            char **envp);

  void clearGlobal();

  void clearMemory();

  void prepareSymbolicValue(
      ExecutionState &state, KInstruction *targetW,
      std::string name = "symbolic_value",
      ref<SymbolicSource> source = SourceBuilder::symbolicValue());

  void prepareSymbolicRegister(ExecutionState &state, StackFrame &sf,
                               unsigned index);

  void prepareSymbolicArgs(ExecutionState &state, KFunction *kf);

  ref<Expr> makeSymbolic(llvm::Value *value, ExecutionState &state,
                         uint64_t size, Expr::Width width,
                         const std::string &name, ref<SymbolicSource> source);

  ref<Expr> makeSymbolicValue(llvm::Value *value, ExecutionState &state,
                              uint64_t size, Expr::Width width,
                              const std::string &name);

  void runFunctionAsMain(llvm::Function *f, int argc, char **argv,
                         char **envp) override;

  /*** Runtime options ***/

  void setHaltExecution(HaltExecution::Reason value) override {
    haltExecution = value;
  }

  void setInhibitForking(bool value) override { inhibitForking = value; }

  void prepareForEarlyExit() override;

  /*** State accessor methods ***/

  unsigned getPathStreamID(const ExecutionState &state) override;

  unsigned getSymbolicPathStreamID(const ExecutionState &state) override;

  void
  getConstraintLog(const ExecutionState &state, std::string &res,
                   Interpreter::LogType logFormat = Interpreter::STP) override;

  void setInitializationGraph(const ExecutionState &state,
                              const Assignment &model, KTest &tc);

  void logState(const ExecutionState &state, int id,
                std::unique_ptr<llvm::raw_fd_ostream> &f) override;

  bool getSymbolicSolution(const ExecutionState &state, KTest &res) override;

  void getCoveredLines(
      const ExecutionState &state,
      std::map<const std::string *, std::set<unsigned>> &res) override;

  Expr::Width getWidthForLLVMType(llvm::Type *type) const;
  size_t getAllocationAlignment(const llvm::Value *allocSite) const;

  /// Returns the errno location in memory of the state
  int *getErrnoLocation(const ExecutionState &state) const;

  MergingSearcher *getMergingSearcher() const { return mergingSearcher; };
  void setMergingSearcher(MergingSearcher *ms) { mergingSearcher = ms; };
  void executeStep(ExecutionState &state);
};

} // namespace klee

#endif /* KLEE_EXECUTOR_H */
