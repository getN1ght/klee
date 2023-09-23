#ifndef EDM_H
#define EDM_H

#include <unordered_set>

namespace klee {

template <typename E> class Notifiable {
  template<typename> friend class Listenable;

protected:
  // virtual void onNotify(Args... &&args) {}
  virtual void onNotify(const E &event) {}
};

template <typename E> class Listenable {
private:
  std::unordered_set<Notifiable<E> *> listeners;

public:
  bool listen(Notifiable<E> *listener) {
    return listeners.insert(listener).second;
  }

  bool leave(Notifiable<E> *listener) { return listeners.erase(listener) != 0; }

  void notifyAll(const E &event) {
    for (const auto &listener : listeners) {
      listener->onNotify(event);
    }
  }
};

} // namespace klee

#endif
