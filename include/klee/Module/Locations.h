//===-- Locations.h ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Classes to represent code locations
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_LOCATIONS_H
#define KLEE_LOCATIONS_H

#include "klee/Module/KModule.h"
#include "klee/Module/InstructionInfoTable.h"


namespace klee {

enum ReachWithError {
  NullPointerException,
  None
};

class Location {
  const std::string filename;
  unsigned line;

public:
  bool isReported;

  Location(const std::string &f, unsigned l) : filename(f), line(l), isReported(false) {}

  bool isTheSameAsIn(KInstruction *instr) const;
  bool isInside(const FunctionInfo &info) const;
  bool isInside(KBlock *block) const;

  std::string toString() const;
};

class Locations {
  using LocationsData = std::vector<Location *>;
  using iterator = LocationsData::const_iterator;
  ReachWithError error;

public:
  LocationsData path;
  Location *start;

  Locations(ReachWithError error) : error(error), start(nullptr) {}
  ~Locations() {
    if (!(isSingleton())) {
      for (auto loc : path)
        delete loc;
    }
    delete start;
  }

  bool isSingleton() const {
    return path.size() == 1 && path.back() == start;
  }

  iterator begin() const { return path.cbegin(); }
  iterator end() const { return path.cend(); }

  ReachWithError targetError() const { return error; }

  void add(const std::string &filename, unsigned lineno) {
    auto loc = new Location(filename, lineno);
    if (start == nullptr) {
      start = loc;
    } else if (isSingleton()) {
      path.pop_back();
    }
    path.push_back(loc);
  }
};

} // End klee namespace

#endif /* KLEE_LOCATIONS_H */
