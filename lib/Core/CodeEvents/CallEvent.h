#ifndef KLEE_CALL_EVENT_H
#define KLEE_CALL_EVENT_H

#include "../CodeEvent.h"
#include "../CodeLocation.h"

#include "klee/ADT/Ref.h"
#include "klee/Module/KModule.h"

#include <string>

namespace klee {

/// @brief
class CallEvent : public CodeEvent {
private:
  /// @brief Called function. Provides additional info
  /// for event description.
  const KFunction *const called;

public:
  explicit CallEvent(const ref<CodeLocation> &location,
                     const KFunction *const called)
      : CodeEvent(EventKind::CALL, location), called(called) {}

  std::string description() const override {
    return std::string("Calling '") + called->getName().str() +
           std::string("()'");
  }

  static bool classof(const CodeEvent *rhs) {
    return rhs->getKind() == EventKind::CALL;
  }
};

} // namespace klee

#endif // KLEE_CALL_EVENT_H
