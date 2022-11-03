#include "klee/Runner/run_klee.h"

/* -*- mode: c++; c-basic-offset: 2; -*- */

//===-- main.cpp ------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include <klee/Misc/json.hpp>
#include <fstream>

using json = nlohmann::json;

using namespace llvm;
using namespace klee;


ReachWithError parseError(json &event) {
  if (!event.contains("event"))
    return ReachWithError::None;
  auto error = event.at("event");
  if (error.empty() || error.at("kind") != "Error")
    return ReachWithError::None;
  std::string errorType = error.at("error").at("sort");
  if ("NullDereference" == errorType)
      return ReachWithError::NullPointerException;
  else //TODO: extent to new types
      return ReachWithError::None;
}


LocatedEvent *parseEvent(json &event) {
  auto location = event.at("location");
  std::string file = location.at("file");
  std::string f = location.at("function");
  unsigned reportLine = location.at("reportLine");
  Location loc(file, f, reportLine);
  return new LocatedEvent(loc, parseError(event));
}

PathForest *parseEvents(json &events) {
  if (events.empty())
    return nullptr;
  auto layer = new PathForest();
  for (auto event : events) {
    auto loc = parseEvent(event);
    auto subTree = event.contains("children") ? parseEvents(event.at("children")) : nullptr;
    layer->addSubTree(loc, subTree);
  }
  return layer;
}

PathForest *parseInputPathTree(const std::string &inputPathTreePath) {
  std::ifstream file(inputPathTreePath);
  json pathForestJSON = json::parse(file);
  return parseEvents(pathForestJSON.at("events"));
}
