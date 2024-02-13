#ifndef KLEE_CODE_EVENT_H
#define KLEE_CODE_EVENT_H

#include "CodeLocation.h"
#include "klee/ADT/Ref.h"

#include "klee/Module/SarifReport.h"

#include <optional>
#include <string>

namespace klee {

/// @brief Base unit for storing information about event from source code.
/// Chain of such units may form a history for particular execution state.
class CodeEvent {
public:
  /// @brief Server for LLVM RTTI purposes.
  const enum EventKind { ALLOC, BR, CALL, ERR, RET } kind;

public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  /// @brief Location information for the event
  const ref<CodeLocation> location;

  explicit CodeEvent(EventKind kind, const ref<CodeLocation> &location)
      : kind(kind), location(location) {}

  /// @brief Additional info to current event.
  /// @return String describing event.
  virtual std::string description() const = 0;

  /// @brief Serialize event to the JSON format.
  /// @return JSON object describing event.
  LocationJson serialize() const {
    LocationJson result;

    result.message = {{description()}};
    result.physicalLocation = {{location->serialize()}};

    return result;
  }

  /// @brief Kind of event used for LLVM RTTI.
  /// @return Kind of event.
  EventKind getKind() const { return kind; }

  virtual ~CodeEvent() = default;
};

} // namespace klee

#endif // KLEE_CODE_EVENT_H
