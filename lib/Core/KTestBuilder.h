#ifndef KLEE_KTESTBUILDER_H
#define KLEE_KTESTBUILDER_H

#include "ConstantAddressSpace.h"
#include "ExecutionState.h"
#include "Memory.h"

#include <klee/ADT/KTest.h>

#include <map>

namespace klee {

class ExecutionState;

class KTestBuilder {
public:
  KTestBuilder(const ExecutionState &state, const Assignment &model);

  KTestBuilder &fillArgcArgv(unsigned argc, char **argv, unsigned symArgc,
                             unsigned symArgv);
  KTestBuilder &fillInitialPointers();
  KTestBuilder &fillInitialContent();
  KTestBuilder &fillFinalPointers();
  KTestBuilder &fillFinalContent();

  KTest build();

private:
  void initialize(const ExecutionState &state);
  std::size_t
  countObjectsFromOrder(const ConstantResolutionList &resolution) const;

private:
  const Assignment &model_;

  std::vector<Symbolic> symbolics;
  ConstantAddressSpace constantAddressSpace_;
  ConstantPointerGraph constantPointerGraph_;
  std::map<ObjectPair, std::size_t> order_;

  // Constructing object
  KTest ktest_;
};

} // namespace klee

#endif
