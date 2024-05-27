#include "Memory.h"

#include "klee/ASContext.h"
#include "klee/AllocationRecord.h"

#include "llvm/Support/ErrorHandling.h"
#include <llvm/Support/Debug.h>

#include <list>

using namespace llvm;

namespace klee {

class MemoryObject;

AllocationRecord::AllocationRecord(const AllocationRecord &other)
    : record(other.record) {}

AllocationRecord &AllocationRecord::operator=(const AllocationRecord &other) {
  if (this != &other) {
    record = other.record;
  }
  return *this;
}

void AllocationRecord::addAddr(ASContext &context, MemoryObject *mo) {
  Entry *entry = find(context);
  if (!entry) {
    ref<ASContext> c(new ASContext(context));
    std::list<ref<MemoryObject>> q;
    q.push_back(mo);
    record.push_back(std::make_pair(c, q));
  } else {
    std::list<ref<MemoryObject>> &q = entry->second;
    q.push_back(mo);
  }
}

ref<MemoryObject> AllocationRecord::getAddr(ASContext &context) {
  Entry *entry = find(context);
  if (entry == NULL) {
    llvm_unreachable("Could not find any entry for the context");
  }

  std::list<ref<MemoryObject>> &q = entry->second;
  if (q.empty()) {
    llvm_unreachable("Could not find any MemoryObject for the context");
  }

  ref<MemoryObject> mo = q.front();
  q.pop_front();

  return mo;
}

bool AllocationRecord::exists(ASContext &context) {
  return find(context) != NULL;
}

AllocationRecord::Entry *AllocationRecord::find(ASContext &context) {
  for (Record::iterator i = record.begin(); i != record.end(); i++) {
    Entry &entry = *i;
    if (*entry.first == context) {
      return &entry;
    }
  }

  return NULL;
}

void AllocationRecord::dump() {
  if (record.empty()) {
    DEBUG_WITH_TYPE(DEBUG_BASIC, klee_message("allocation record is empty"));
  } else {
    DEBUG_WITH_TYPE(DEBUG_BASIC, klee_message("allocation record:"));
    for (Record::iterator i = record.begin(); i != record.end(); i++) {
      Entry &entry = *i;

      /* dump context */
      ref<ASContext> c = entry.first;
      c->dump();

      /* dump addresses */
      MemoryObjectList &memoryObjects = entry.second;
      DEBUG_WITH_TYPE(DEBUG_BASIC, klee_message("memory objects:"));
      for (MemoryObjectList::iterator j = memoryObjects.begin();
           j != memoryObjects.end(); j++) {
        ref<MemoryObject> mo = *j;
        if (mo) {
          DEBUG_WITH_TYPE(DEBUG_BASIC, klee_message("-- %lx", mo->address));
        } else {
          DEBUG_WITH_TYPE(DEBUG_BASIC, klee_message("-- null"));
        }
      }
    }
  }
}

} // namespace klee
