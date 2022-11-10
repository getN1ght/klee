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

#include "klee/Support/ErrorHandling.h"

#include <klee/Misc/json.hpp>
#include <fstream>

using json = nlohmann::json;

using namespace llvm;
using namespace klee;


class Frames {

using FramesMtd = std::vector<std::pair<std::string, std::string> >;
FramesMtd frames;
std::string functionShouldBeCalled = "";

void checkConsistency(const std::string &function) {
  if (frames.empty())
    return;
  if (functionShouldBeCalled.empty())
    functionShouldBeCalled = frames.back().second;
  if (functionShouldBeCalled != function) {
    klee_warning("JSON inconsistency: trace event has function %s but should have function %s",
      function.c_str(), functionShouldBeCalled.c_str());
  }
  functionShouldBeCalled = "";
}

void ensureAtLeastOneFrame(json &loc) {
  std::string function = loc.at("function");
  checkConsistency(function);
  if (frames.empty())
    frames.emplace_back(loc.at("file"), function);
}

public:
void enterCall(json &loc, const std::string &function) {
    ensureAtLeastOneFrame(loc);
    functionShouldBeCalled = function;
}

void exitCall(json &loc, const std::string &function) {
  if (!frames.empty()) {
    auto oldFunc = frames.back().second;
    if (function == oldFunc) {
      frames.pop_back();
    } else {
      klee_warning("JSON inconsistency: trace event with function %s is before an event with an exit from %s",
        oldFunc.c_str(), function.c_str());
    }
  }
  ensureAtLeastOneFrame(loc);
}

void handleNonCallEvent(json &loc) {
  ensureAtLeastOneFrame(loc);
}

FramesMtd::iterator entryPoint() {
  return frames.begin();
}

}; // class Frames

class TraceParser {

struct cmpLocatedEvent {
    bool operator()(const LocatedEvent& a, const LocatedEvent& b) const {
        return a.location < b.location;
    }
};

std::map<LocatedEvent, LocatedEvent *, cmpLocatedEvent> locatedEvents;

LocatedEvent *locatedEvent(LocatedEvent *event) {
  if (event->error != ReachWithError::None)
    return event;
  auto it = locatedEvents.find(*event);
  if (it == locatedEvents.end()) {
    locatedEvents.insert(std::make_pair(*event, event));
    return event;
  } else {
    delete event;
    return it->second;
  }
}

ReachWithError parseError(json &traceEvent) {
  if (!traceEvent.contains("event"))
    return ReachWithError::None;
  auto event = traceEvent.at("event");
  if (event.empty() || event.at("kind") != "Error")
    return ReachWithError::None;
  std::string errorType = event.at("error").at("sort");
  if ("NullDereference" == errorType)
      return ReachWithError::NullPointerException;
  else if ("CheckAfterDeref" == errorType)
      return ReachWithError::NullCheckAfterDerefException;
  else //TODO: extent to new types
      return ReachWithError::None;
}

LocatedEvent *parseTraceEvent(json &traceEvent) {
  auto location = traceEvent.at("location");
  std::string file = location.at("file");
  unsigned reportLine = location.at("reportLine");
  auto error = parseError(traceEvent);
  LocatedEvent *le = nullptr;
  if (location.contains("instructionOffset")) {
    unsigned instructionOffset = location.at("instructionOffset");
    std::string function = location.at("function");
    le = new LocatedEvent(Location(file, function, reportLine, instructionOffset), error);
  } else {
    le = new LocatedEvent(Location(file, reportLine), error);
  }
  return locatedEvent(le);
}

LocatedEvent *deduceEntryPoint(json &trace) {
  Frames frames;
  for (auto traceEvent : trace) {
    auto loc = traceEvent.at("location");
    auto event = traceEvent.at("event");
    auto eventKind = event.at("kind");
    if ("ExitCall" == eventKind) {
      frames.exitCall(loc, event.at("function"));
      continue;
    }
    if ("EnterCall" == eventKind) {
      frames.enterCall(loc, event.at("function"));
      continue;
    }
    frames.handleNonCallEvent(loc);
  }
  auto p = frames.entryPoint();
  auto le = new LocatedEvent(Location(p->first, p->second), ReachWithError::None);
  // llvm::errs() << "Deduced entry point: " << p->first << ' ' << p->second << '\n';
  return locatedEvent(le);
}

std::vector<LocatedEvent *> *parseTrace(json &trace, unsigned id) {
  auto out = new std::vector<LocatedEvent *>();
  out->reserve(trace.size() + 1);
  auto entryPoint = deduceEntryPoint(trace);
  out->push_back(entryPoint);
  for (auto traceEvent : trace) {
    auto le = parseTraceEvent(traceEvent);
    if (le->error != ReachWithError::None)
      le->id = id;
    out->push_back(le);
  }
  return out;
}

public:
TraceParser() {}

PathForest *parseErrors(json &errors) {
  auto layer = new PathForest();
  if (errors.empty())
    return layer;
  for (auto errorTrace : errors) {
    unsigned id = errorTrace.at("id");
    auto trace = parseTrace(errorTrace.at("trace"), id);
    layer->addTrace(trace);
    delete trace;
  }
  return layer;
}

}; // class TraceParser

PathForest *parseInputPathTree(const std::string &inputPathTreePath) {
  std::ifstream file(inputPathTreePath);
  if (file.fail())
    klee_error("Cannot read file %s", inputPathTreePath.c_str());
  json pathForestJSON = json::parse(file);
  TraceParser tp;
  return tp.parseErrors(pathForestJSON.at("errors"));
}
