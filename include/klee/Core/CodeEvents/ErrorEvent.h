#ifndef KLEE_ERROR_EVENT_H
#define KLEE_ERROR_EVENT_H

#include "klee/Core/CodeEvent.h"
#include "klee/Core/TerminationTypes.h"

#include <optional>
#include <string>

namespace klee {

struct ErrorEvent : public CodeEvent {
public:
  const StateTerminationType ruleID;
  const std::string message;

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

#endif
