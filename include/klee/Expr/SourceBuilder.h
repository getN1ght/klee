#ifndef KLEE_SOURCEBUILDER_H
#define KLEE_SOURCEBUILDER_H

#include "SymbolicSource.h"
#include <unordered_map>

namespace klee {

class SourceBuilder {
private:
  ref<SymbolicSource> constantSource;
  ref<SymbolicSource> makeSymbolicSource;

public:
  SourceBuilder();

  SymbolicSource *constant();
  SymbolicSource *makeSymbolic();
};

};

#endif /* KLEE_EXPRBUILDER_H */
