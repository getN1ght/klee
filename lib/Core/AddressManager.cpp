#include "AddressManager.h"

#include "klee/Expr/Expr.h"
#include "MemoryManager.h"

namespace klee {

void AddressManager::addAllocation(const Array *array, IDType id) {
  bindingsArraysToObjects[array] = id;
}

void *AddressManager::allocate(const Array *array, uint64_t size) {
  IDType id = bindingsArraysToObjects.at(array);

  const auto &objects = memory->getAllocatedObjects(id);
  auto sizeLocation = objects.lower_bound(size);
  MemoryObject *newMO;
  if (sizeLocation == objects.end()) {
    uint64_t newSize =
        1 << (sizeof(size) - __builtin_clz(std::max((uint64_t)1, size)));
    MemoryObject *mo = std::prev(objects.end())->second;
    newMO = memory->allocate(newSize, mo->isLocal, mo->isGlobal, mo->allocSite,
                             /*FIXME:*/ 8, mo->addressExpr, mo->sizeExpr,
                             mo->lazyInitializationSource, mo->timestamp);
    newMO->id = mo->id;
  } else {
    newMO = sizeLocation->second;
  }
  return reinterpret_cast<void *>(newMO->address);
}

MemoryObject *AddressManager::allocateMemoryObject(const Array *array,
                                                   uint64_t size) {
  IDType id = bindingsArraysToObjects.at(array);
  const auto &objects = memory->getAllocatedObjects(id);
  auto resultIterator = objects.lower_bound(size);
  assert(resultIterator != objects.end());
  return resultIterator->second;
}

} // namespace klee