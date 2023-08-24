#ifndef _ADDRESS_GENERATOR_H
#define _ADDRESS_GENERATOR_H

#include "klee/ADT/Ref.h"

#include <cstdint>

namespace klee {
class Expr;


typedef uint64_t AddressSpaceContext;

class AddressGenerator {
public:
  AddressSpaceContext ctx;

  virtual void *allocate(ref<Expr>, uint64_t size) = 0;
  virtual ~AddressGenerator() = default;

  void inContext(const AddressSpaceContext &newCtx) {
    ctx = newCtx;
  }
};
}; // namespace klee

#endif
