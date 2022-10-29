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


namespace klee {

enum ReachWithError {
  NullPointerException,
  None
};

class Locations {
  using LocationsData = std::vector<std::pair<std::string, unsigned>>;
  using iterator = LocationsData::const_iterator;
  LocationsData locations;
  ReachWithError error;

public:
  Locations(ReachWithError error) : error(error) {}
  iterator begin() const { return locations.cbegin(); }
  iterator end() const { return locations.cend(); }

  ReachWithError targetError() const { return error; }

  void add(const std::string &filename, unsigned lineno) {
    locations.push_back(std::make_pair(filename, lineno));
  }
};

struct ResolvedLocations {
  using LocationsData = std::vector<std::unordered_set<KBlock *>>;
  LocationsData locations;
  ResolvedLocations() {}
  ResolvedLocations(const KModule *kmodule, const Locations &locs);
};

} // End klee namespace

#endif /* KLEE_LOCATIONS_H */
