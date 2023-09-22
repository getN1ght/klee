#ifndef BIMAP_H
#define BIMAP_H

#include <functional>
#include <iterator>
#include <map>

namespace klee {

/*
 * Bidirectional map. Stores added values by keys
 * and additionally keys by values. Inserting
 */
template <typename K, typename V, typename CK = std::less<K>,
          typename CV = std::less<V>>
class BiMap {
private:
  typedef typename std::map<K, V, CK>::const_iterator const_iterator;
  std::map<K, V, CK> data;
  std::map<V, K, CV> rdata;

public:
  size_t size() const { return data.size(); }
  bool empty() const { return size() == 0; }

  bool put(const K &key, const V &value) {
    if (containsKey(key) || containsValue(value)) {
      return false;
    }
    data.emplace(key, value);
    rdata.emplace(value, key);
    return true;
  }

  bool eraseByKey(const K &key) {
    if (containsKey(key)) {
      return false;
    }
    const V &value = getByKey(key);
    data.erase(key);
    rdata.erase(value);
    return true;
  }

  bool eraseByValue(const V &value) {
    if (!containsValue(value)) {
      return false;
    }
    const K &key = getByValue(value);
    data.erase(key);
    rdata.erase(value);
    return true;
  }

  bool containsKey(const K &key) { return data.count(key) != 0; }

  bool containsValue(const V &value) { return rdata.count(value) != 0; }

  [[nodiscard]] const V &getByKey(const K &key) { return data.at(key); }

  [[nodiscard]] const K &getByValue(const V &vKey) { return rdata.at(vKey); }

  const_iterator begin() const { return data.begin(); }
  const_iterator end() const { return data.end(); }
};

template <typename V>
using BiArray = BiMap<uint64_t, V, std::less<uint64_t>, std::less<V>>;

} // namespace klee

#endif