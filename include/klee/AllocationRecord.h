#ifndef KLEE_ALLOCATION_RECORD_H
#define KLEE_ALLOCATION_RECORD_H

#include "klee/ADT/Ref.h"
#include "klee/ASContext.h"

#include <list>

namespace klee {

class MemoryObject;

class AllocationRecord {
public:
  AllocationRecord() {}

  AllocationRecord(const AllocationRecord &other);

  AllocationRecord &operator=(const AllocationRecord &other);

  ~AllocationRecord() = default;

  void addAddr(ASContext &context, MemoryObject *mo);

  ref<MemoryObject> getAddr(ASContext &context);

  bool exists(ASContext &context);

  void dump();

private:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  /*  TODO: change ref<ASContext> to ASContext? */
  typedef std::list<ref<MemoryObject>> MemoryObjectList;
  typedef std::pair<ref<ASContext>, MemoryObjectList> Entry;
  typedef std::vector<Entry> Record;

  Entry *find(ASContext &context);

  Record record;
};

} // namespace klee

#endif
