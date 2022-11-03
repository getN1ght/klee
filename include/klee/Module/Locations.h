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
  unsigned line;
  const std::string function;

public:
  const std::string filename;

  Location(const std::string &f, const std::string &function, unsigned l)
    : line(l), function(function), filename(f) {}

  bool isTheSameAsIn(KInstruction *instr) const;
  bool isInside(const FunctionInfo &info) const;
  bool isInside(KBlock *block) const;

  std::string toString() const;
};

struct LocatedEvent {
  Location location;
  ReachWithError error;
  bool isReported;

  LocatedEvent(Location location, ReachWithError error) : location(location), error(error), isReported(false) {}

  std::string toString() const;
};

struct PathForest {
  std::unordered_map<LocatedEvent *, PathForest *> layer;
  ~PathForest();

  void addSubTree(LocatedEvent *loc, PathForest *subTree);
  void addLeaf(LocatedEvent *loc);

  bool empty() const;

  /// @brief Sets singleton paths to size two
  void normalize();
};

} // End klee namespace

#endif /* KLEE_LOCATIONS_H */
