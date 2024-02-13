#ifndef KLEE_ERROR_EVENT_H
#define KLEE_ERROR_EVENT_H

#include "../CodeEvent.h"
#include "klee/Core/TerminationTypes.h"

#include <optional>
#include <string>

namespace klee {

struct ErrorEvent : public CodeEvent {
public:
  /// @brief ID for this error.
  const StateTerminationType ruleID;

  /// @brief Message describing this error.
  const std::string message;

  /// @brief Event associated with this error
  /// which may be treated as a "source" of error
  /// (e.g. memory allocation for Out-Of-Bounds error).
  const std::optional<ref<CodeEvent>> source;

  ErrorEvent(const ref<CodeEvent> &source, const ref<CodeLocation> &sink,
             StateTerminationType ruleID, const std::string &message)
      : CodeEvent(EventKind::ERR, sink), ruleID(ruleID), message(message),
        source(source) {}

  ErrorEvent(const ref<CodeLocation> &sink, StateTerminationType ruleID,
             const std::string &message)
      : CodeEvent(EventKind::ERR, sink), ruleID(ruleID), message(message),
        source(std::nullopt) {}

  std::string description() const override { return message; }

  static bool classof(const CodeEvent *rhs) {
    return rhs->getKind() == EventKind::ERR;
  }
};

} // namespace klee

#endif // KLEE_ERROR_EVENT_H
