#ifndef KLEE_SOURCEBUILDER_H
#define KLEE_SOURCEBUILDER_H

#include "SymbolicSource.h"
#include <unordered_map>

namespace klee {

class SourceBuilder {
public:
  ref<ArraySource> constant() const;
  ref<ArraySource> makeSymbolic() const;
  ref<ArraySource> symbolicAddress() const;
  ref<ArraySource> symbolicSize() const;
  ref<ArraySource> lazyInitializationMakeSymbolic() const;
};

};

#endif /* KLEE_EXPRBUILDER_H */
