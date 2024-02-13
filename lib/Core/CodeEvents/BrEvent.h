#ifndef KLEE_BR_EVENT_H
#define KLEE_BR_EVENT_H

#include "../CodeEvent.h"
#include "klee/ADT/Ref.h"

#include <string>

namespace klee {

class BrEvent : public CodeEvent {
private:
  /// @brief Described chosen branch event: `true` if
  /// `then`-branch was chosen and `false` otherwise.
  bool chosenBranch = true;

public:
  explicit BrEvent(const ref<CodeLocation> &location)
      : CodeEvent(EventKind::BR, location) {}

  /// @brief Modifies chosen branch for `this` event.
  /// @param branch true if condition in chosen branch is true and false
  /// otherwise.
  /// @return Reference to this object.
  /// @note This function does not return modified copy of this object; it
  /// returns *this* object.
  BrEvent &withBranch(bool branch) {
    chosenBranch = branch;
    return *this;
  }

  std::string description() const override {
    return std::string("Choosing ") +
           (chosenBranch ? std::string("true") : std::string("false")) +
           std::string(" branch");
  }

  static bool classof(const CodeEvent *rhs) {
    return rhs->getKind() == EventKind::BR;
  }
};

} // namespace klee

#endif // KLEE_BR_EVENT_H
