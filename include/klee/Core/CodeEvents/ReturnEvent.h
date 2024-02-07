#ifndef KLEE_RETURN_EVENT_H
#define KLEE_RETURN_EVENT_H

#include "klee/Core/CodeEvent.h"
#include "klee/Core/CodeLocation.h"
#include "klee/Module/KModule.h"

#include <string>

namespace klee {

class ReturnEvent : public CodeEvent {
private:
  const KFunction *const caller;

public:
  explicit ReturnEvent(const ref<CodeLocation> &location,
                       const KFunction *const caller)
      : CodeEvent(EventKind::RET, location), caller(caller) {}

  std::string description() const override {
    return std::string("Returning to '") + caller->getName().str() +
           std::string("()'");
  }

  static bool classof(const CodeEvent *rhs) {
    return rhs->getKind() == EventKind::RET;
  }
};

} // namespace klee

#endif
