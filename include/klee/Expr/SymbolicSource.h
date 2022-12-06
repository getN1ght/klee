#ifndef KLEE_SYMBOLICSOURCE_H
#define KLEE_SYMBOLICSOURCE_H

#include "klee/ADT/Ref.h"
#include <string>

namespace klee {

class Array;

class SymbolicSource {
public:
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  enum class Kind {
    Constant,
    ConstantWithSymbolicSize,
    LazyInitializationSymbolic,
    MakeSymbolic,
    SymbolicAddress,
    SymbolicSize
  };

public:
  virtual ~SymbolicSource() {}
  virtual Kind getKind() const = 0;
  virtual std::string getName() const = 0;
  virtual bool isSymcrete() const = 0;

  virtual int compare(const SymbolicSource &another) {
    return getKind() == another.getKind();
  }

  static bool classof(const SymbolicSource *) { return true; }
};

class ConstantSource : public SymbolicSource {
public:
  Kind getKind() const override { return Kind::Constant; }
  virtual std::string getName() const override { return "constant"; }
  virtual bool isSymcrete() const override { return false; }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::Constant;
  }
  static bool classof(const ConstantSource *) { return true; }
};

class ConstantWithSymbolicSizeSource : public SymbolicSource {
public:
  Kind getKind() const override { return Kind::ConstantWithSymbolicSize; }
  virtual std::string getName() const override { return "constant"; }
  virtual bool isSymcrete() const override { return false; }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::ConstantWithSymbolicSize;
  }
  static bool classof(const ConstantWithSymbolicSizeSource *) { return true; }
};

class MakeSymbolicSource : public SymbolicSource {
public:
  Kind getKind() const override { return Kind::MakeSymbolic; }
  virtual std::string getName() const override { return "symbolic"; }
  virtual bool isSymcrete() const override { return false; }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::MakeSymbolic;
  }
  static bool classof(const MakeSymbolicSource *) { return true; }
};

class SymbolicAllocationSource : public SymbolicSource {
public:
  const Array *linkedArray;

  int compare(const SymbolicSource &another) {
    if (getKind() != another.getKind()) {
      return getKind() < another.getKind() ? -1 : 1;
    }
    const SymbolicAllocationSource &anotherCasted =
        static_cast<const SymbolicAllocationSource &>(another);
    if (linkedArray == anotherCasted.linkedArray) {
      return 0;
    }
    return linkedArray < anotherCasted.linkedArray ? -1 : 1;
  }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::SymbolicAddress ||
           S->getKind() == Kind::SymbolicSize;
  }
};

class SymbolicAddressSource : public SymbolicAllocationSource {
public:
  Kind getKind() const override { return Kind::SymbolicAddress; }
  virtual std::string getName() const override { return "symbolicAddress"; }
  virtual bool isSymcrete() const override { return true; }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::SymbolicAddress;
  }
  static bool classof(const SymbolicAddressSource *) { return true; }
};

class SymbolicSizeSource : public SymbolicAllocationSource {
public:
  Kind getKind() const override { return Kind::SymbolicSize; }
  virtual std::string getName() const override { return "symbolicSize"; }
  virtual bool isSymcrete() const override { return true; }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::SymbolicSize;
  }
  static bool classof(const SymbolicSizeSource *) { return true; }
};

class LazyInitializationSymbolicSource : public SymbolicSource {
public:
  Kind getKind() const override { return Kind::LazyInitializationSymbolic; }
  virtual std::string getName() const override {
    return "lazyInitializationMakeSymbolic";
  }
  virtual bool isSymcrete() const override { return false; }

  static bool classof(const SymbolicSource *S) {
    return S->getKind() == Kind::LazyInitializationSymbolic;
  }
  static bool classof(const LazyInitializationSymbolicSource *) { return true; }
};

} // namespace klee

#endif /* KLEE_SYMBOLICSOURCE_H */
