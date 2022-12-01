#include "klee/Expr/SourceBuilder.h"
#include "klee/Expr/SymbolicSource.h"
#include "klee/Expr/Expr.h"

using namespace klee;

SourceBuilder::SourceBuilder() {
  constantSource = ref<SymbolicSource>(new ConstantSource());
  makeSymbolicSource = ref<SymbolicSource>(new MakeSymbolicSource());
}

SymbolicSource *SourceBuilder::constant() {
  return constantSource.get();
}

SymbolicSource *SourceBuilder::makeSymbolic() {
  return makeSymbolicSource.get();
}
