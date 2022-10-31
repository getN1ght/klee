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
  Location(const std::string &f, unsigned l) : filename(f), line(l) {}

  bool isTheSameAsIn(KInstruction *instr) const;
  bool isInside(const FunctionInfo &info) const;
  bool isInside(KBlock *block) const;
};

class Locations {
  using LocationsData = std::vector<Location *>;
  using iterator = LocationsData::const_iterator;
  LocationsData locations;
  ReachWithError error;

public:
  Locations(ReachWithError error) : error(error) {}
  ~Locations() {
    for (auto loc : locations)
      delete loc;
  }

  iterator begin() const { return locations.cbegin(); }
  iterator end() const { return locations.cend(); }

  ReachWithError targetError() const { return error; }

  void add(const std::string &filename, unsigned lineno) {
    locations.push_back(new Location(filename, lineno));
  }
};

struct ResolvedLocation {
  Location *originalLocation;
  std::unordered_set<KBlock *> blocks;

  ResolvedLocation(Location *loc, std::unordered_set<KBlock *> &blocks) : originalLocation(loc), blocks(blocks) {}
};

struct ResolvedLocations {
  using LocationsData = std::vector<ResolvedLocation>;
  LocationsData locations;
  ResolvedLocations() {}
  ResolvedLocations(const KModule *kmodule, const Locations *locs);
};

} // End klee namespace

#endif /* KLEE_LOCATIONS_H */
