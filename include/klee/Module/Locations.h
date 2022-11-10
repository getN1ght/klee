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

#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"
#include "klee/Module/InstructionInfoTable.h"


namespace klee {

enum ReachWithError {
  NullPointerException,
  NullCheckAfterDerefException,
  None
};

class Location {
  KInstruction *instruction = nullptr;
  unsigned line;

public:
  unsigned offset;
  const std::string function;
  const std::string filename;

  Location(const std::string &f, const std::string &function, unsigned line, unsigned offset)
    : line(line), offset(offset), function(function), filename(f) {}
  Location(const std::string &f, const std::string &function)
    : Location(f, function, 0, 0) {}
  Location(const std::string &filename, unsigned line)
    : Location(filename, "", line, 0) {}

  bool isTheSameAsIn(KInstruction *instr) const;
  bool isInside(const FunctionInfo &info) const;
  bool isInside(KBlock *block) const;

  std::string toString() const;

  bool hasFunctionWithOffset() const {
    return !function.empty();
  }

  KInstruction *initInstruction(KModule *module);

  friend bool operator <(const Location& lhs, const Location& rhs) {
    return
           lhs.line < rhs.line     ||     (lhs.line == rhs.line &&
        (lhs.offset < rhs.offset   ||   (lhs.offset == rhs.offset &&
      (lhs.filename < rhs.filename || (lhs.filename == rhs.filename &&
      (lhs.function < rhs.function))))));
  }
};

struct LocatedEvent {
  Location location;
  ReachWithError error;
  unsigned id = 0;
  bool isReported;

  LocatedEvent(Location location, ReachWithError error) : location(location), error(error), isReported(false) {}

  bool hasFunctionWithOffset() { return location.hasFunctionWithOffset(); }

  std::string toString() const;
};

struct PathForest {
  std::unordered_map<LocatedEvent *, PathForest *> layer;
  ~PathForest();

  void addSubTree(LocatedEvent *loc, PathForest *subTree);
  void addLeaf(LocatedEvent *loc);

  void addTrace(std::vector<LocatedEvent *> *trace);

  bool empty() const;

  /// @brief Sets singleton paths to size two
  void normalize();
};

} // End klee namespace

#endif /* KLEE_LOCATIONS_H */
