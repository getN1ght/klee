#ifndef KLEE_EVENT_LIST_H
#define KLEE_EVENT_LIST_H

#include "klee/Expr/Path.h"

namespace klee {

template <typename T> class ref;
template <typename T> class ImmutableList;
class CodeFlowJson;
class CodeEvent;

/// @brief Class capable of storing code events
/// and serializing them into SARIF format.
class EventRecorder {
private:
  /// @brief Inner storage of code events.
  ImmutableList<ref<CodeEvent>> events;

public:
  EventRecorder() = default;

  EventRecorder(const EventRecorder &);
  EventRecorder &operator=(const EventRecorder &);
  ~EventRecorder() = default;

  /// @brief Remembers event.
  /// @param event event to record.
  void record(const ref<CodeEvent> &event);

  /// @brief
  /// @param rhs
  void append(const EventRecorder &rhs);

  /// @brief Returns events from the history in specified range.
  /// @param begin range begin.
  /// @param end range end.
  /// @return immutable list containing recorder events from inner storage.
  EventRecorder inRange(const Path::PathIndex &begin,
                        const Path::PathIndex &end) const;

  /// @brief Returns events from the history from the givent event.
  /// @param begin range begin.
  /// @return immutable list containing recorder events from inner storage.
  EventRecorder tail(const Path::PathIndex &begin) const;

  /// @brief
  /// @return
  ref<CodeEvent> last() const;

  /// @brief
  /// @return
  bool empty() const;

  /// @brief Serializes events in this recorder to SARIF report.
  /// @return Structure ready for wrapping into json
  /// (i.e. `json(serialize())`) and serialization.
  CodeFlowJson serialize() const;
};

} // namespace klee

#endif // KLEE_EVENT_LIST_H
