#include "klee/Expr/SourceBuilder.h"

#include "klee/Expr/SymbolicSource.h"
#include "klee/Expr/Expr.h"

using namespace klee;

ref<ArraySource> SourceBuilder::constant() const{
  return new ConstantSource();
}

ref<ArraySource> SourceBuilder::makeSymbolic() const{
  return new MakeSymbolicSource();
}

ref<ArraySource> SourceBuilder::symbolicAddress() const {
  return new SymbolicAddressSource();
}

ref<ArraySource> SourceBuilder::symbolicSize() const {
  return new SymbolicSizeSource();
}

ref<ArraySource> SourceBuilder::lazyInitializationMakeSymbolic() const {
  return new LazyInitializationSymbolicSource();
}
