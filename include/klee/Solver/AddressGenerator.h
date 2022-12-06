#ifndef _ADDRESS_GENERATOR_H
#define _ADDRESS_GENERATOR_H

#include <cstdint>

namespace klee {
class Array;

class AddressGenerator {
public:
  virtual void *allocate(const Array *array, uint64_t size) = 0;
};
};

#endif