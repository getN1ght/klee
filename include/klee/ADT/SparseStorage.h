#ifndef KLEE_SPARSESTORAGE_H
#define KLEE_SPARSESTORAGE_H

#include <unordered_map>

namespace klee {

template <typename KeyType, typename ValueType> class SparseStorage {
private:
  std::unordered_map<KeyType, ValueType> internalStorage;
  const ValueType defaultValue;

  /* FIXME: Can be replaced with internalStorage.contains() with C++20 API */
  bool contains(const KeyType &key) const {
    return internalStorage.count(key) != 0;
  }

public:
  
  class SparseStorageProxy {
  friend class SparseStorage;
  private:
    SparseStorage &owner;
    const KeyType &key;
    SparseStorageProxy(SparseStorage &owner, const KeyType &key)
        : owner(owner), key(key) {}

    SparseStorageProxy(const SparseStorageProxy &other)
        : owner(other.owner), key(other.key) {}
    SparseStorageProxy(const SparseStorageProxy &&other)
        : owner(other.owner), key(other.key) {}

  public:
    
    /* Copy assignment operators */
    SparseStorageProxy &operator=(const SparseStorageProxy &other) {
      return (*this) = static_cast<ValueType &>(other);
    }
    SparseStorageProxy &operator=(SparseStorageProxy &&other) {
      return (*this) = static_cast<ValueType &&>(other);
    }

    /* Copy values operators */
    SparseStorageProxy &operator=(const ValueType &value) {
      owner.internalStorage[key] = value;
      return *this;
    }
    SparseStorageProxy &operator=(ValueType &&value) {
      owner.internalStorage[key] = value;
      return *this;
    }

    operator ValueType() const {
      return owner.contains(key) ? owner.internalStorage.at(key)
                                 : owner.defaultValue;
    }
  };

  SparseStorage(const ValueType &defaultValue) : defaultValue(defaultValue) {}

  SparseStorage(const std::vector<ValueType> &values,
                const ValueType &defaultValue);

  SparseStorageProxy operator[](const KeyType &key) {
    return SparseStorageProxy(*this, key);
  }

  const SparseStorageProxy operator[](const KeyType &key) const {
    return SparseStorageProxy(*this, key);
  }
};

} // namespace klee

#endif