#ifndef KLEE_RETURN_EVENT_H
#define KLEE_RETURN_EVENT_H

#include "../CodeEvent.h"
#include "../CodeLocation.h"
#include "klee/Module/KModule.h"

#include <string>

namespace klee {

class ReturnEvent : public CodeEvent {
private:
  /// @brief Function to which control flow returns.
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

#endif // KLEE_RETURN_EVENT_H
