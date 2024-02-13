#ifndef KLEE_ALLOC_EVENT_H
#define KLEE_ALLOC_EVENT_H

#include "../CodeEvent.h"
#include "klee/Module/KValue.h"

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
DISABLE_WARNING_POP

#include <string>

namespace klee {

struct AllocEvent : public CodeEvent {
  AllocEvent(ref<CodeLocation> location)
      : CodeEvent(EventKind::ALLOC, location) {}

  std::string description() const override {
    const KValue *source = location->source;
    if (llvm::isa<KGlobalVariable>(source)) {
      return std::string("Global memory allocation");
    }

    if (llvm::isa<llvm::AllocaInst>(source->unwrap())) {
      return std::string("Local memory allocation");
    }

    return std::string("Heap memory allocation");
  }

  static bool classof(const CodeEvent *rhs) {
    return rhs->getKind() == EventKind::ALLOC;
  }
};

} // namespace klee

#endif
