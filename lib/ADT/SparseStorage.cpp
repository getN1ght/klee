#include "klee/ADT/SparseStorage.h"
#include "klee/Expr/Expr.h"

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"
DISABLE_WARNING_DEPRECATED_DECLARATIONS
DISABLE_WARNING_POP

namespace klee {
template <>
void SparseStorage<bool>::print(llvm::raw_ostream &os, Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": " << llvm::utostr(element.second);
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      os << llvm::utostr(load(i));
      firstPrinted = true;
    }
    os << "] default: ";
  }
  os << llvm::utostr(defaultValue);
}

template <>
void SparseStorage<unsigned char>::print(llvm::raw_ostream &os,
                                         Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": " << llvm::utostr(element.second);
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      os << llvm::utostr(load(i));
      firstPrinted = true;
    }
    os << "] default: ";
  }
  os << llvm::utostr(defaultValue);
}

template <>
void SparseStorage<ref<ConstantExpr>>::print(llvm::raw_ostream &os,
                                             Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": ";
      if (element.second) {
        os << element.second;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      auto expr = load(i);
      if (expr) {
        os << expr;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "] default: ";
  }
  if (defaultValue) {
    os << defaultValue;
  } else {
    os << "null";
  }
}

template <>
void klee::SparseStorage<
    klee::ref<klee::Expr>,
    klee::OptionalRefEq<klee::Expr>>::print(llvm::raw_ostream &os,
                                            Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": ";
      if (element.second) {
        os << element.second;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      auto expr = load(i);
      if (expr) {
        os << expr;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "] default: ";
  }
  if (defaultValue) {
    os << defaultValue;
  } else {
    os << "null";
  }
}

template <>
void SparseStorageImpl<bool>::print(llvm::raw_ostream &os, Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": " << llvm::utostr(element.second);
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      os << llvm::utostr(load(i));
      firstPrinted = true;
    }
    os << "] default: ";
  }
  os << llvm::utostr(defaultValue);
}

template <>
void SparseStorageImpl<unsigned char>::print(llvm::raw_ostream &os,
                                             Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": " << llvm::utostr(element.second);
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      os << llvm::utostr(load(i));
      firstPrinted = true;
    }
    os << "] default: ";
  }
  os << llvm::utostr(defaultValue);
}

template <>
void SparseStorageImpl<ref<ConstantExpr>>::print(llvm::raw_ostream &os,
                                                 Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": ";
      if (element.second) {
        os << element.second;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      auto expr = load(i);
      if (expr) {
        os << expr;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "] default: ";
  }
  if (defaultValue) {
    os << defaultValue;
  } else {
    os << "null";
  }
}

template <>
void klee::SparseStorageImpl<
    klee::ref<klee::Expr>,
    klee::OptionalRefEq<klee::Expr>>::print(llvm::raw_ostream &os,
                                            Density d) const {
  if (d == Density::Sparse) {
    // "Sparse representation"
    os << "{";
    bool firstPrinted = false;
    auto ordered = calculateOrderedStorage();
    for (const auto &element : ordered) {
      if (firstPrinted) {
        os << ", ";
      }
      os << element.first << ": ";
      if (element.second) {
        os << element.second;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "} default: ";
  } else {
    // "Dense representation"
    os << "[";
    bool firstPrinted = false;
    for (size_t i = 0; i < sizeOfSetRange(); i++) {
      if (firstPrinted) {
        os << ", ";
      }
      auto expr = load(i);
      if (expr) {
        os << expr;
      } else {
        os << "null";
      }
      firstPrinted = true;
    }
    os << "] default: ";
  }
  if (defaultValue) {
    os << defaultValue;
  } else {
    os << "null";
  }
}

} // namespace klee
