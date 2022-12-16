#include <unordered_map>

namespace klee {
class Array;

class AddressGenerator {
public:
  virtual void *allocate(const Array *array, uint64_t size) = 0;
};
};