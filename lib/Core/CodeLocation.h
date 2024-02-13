#ifndef KLEE_CODE_LOCATION_H
#define KLEE_CODE_LOCATION_H

#include "klee/ADT/Ref.h"
#include "klee/Expr/Path.h"
#include "klee/Module/LocationInfo.h"

#include <cstdint>
#include <iostream>
#include <optional>
#include <string>

namespace klee {

struct PhysicalLocationJson;
struct KValue;

/// @brief Represents the location in source code with additional
/// information about place in the state's path.
struct CodeLocation {
  /// @brief Required by klee::ref-managed objects
  class ReferenceCounter _refCount;

  /// @brief Path index in `Path`.
  const Path::PathIndex pathIndex;

  /// @brief Corresponding llvm entity.
  const KValue *source;

  /// @brief Location in source code.
  const LocationInfo location;

private:
  CodeLocation(const Path::PathIndex &pathIndex, const KValue *source,
               const std::string &sourceFilename, uint64_t sourceCodeLine,
               std::optional<uint64_t> sourceCodeColumn)
      : pathIndex(pathIndex), source(source),
        location(
            LocationInfo{sourceFilename, sourceCodeLine, sourceCodeColumn}) {}

  CodeLocation(const CodeLocation &) = delete;
  CodeLocation &operator=(const CodeLocation &) = delete;

public:
  /// @brief: Factory method. Wraps constructed objects in the ref
  /// to provide zero-cost copying of code locations.
  /// @param sourceFilename
  /// @param sourceCodeLine
  /// @param sourceCodeColumn
  /// @return
  static ref<CodeLocation>
  create(const Path::PathIndex &pathIndex, const KValue *source,
         const std::string &sourceFilename, uint64_t sourceCodeLine,
         std::optional<uint64_t> sourceCodeColumn = std::nullopt) {
    return new CodeLocation(pathIndex, source, sourceFilename, sourceCodeLine,
                            sourceCodeColumn);
  }

  /// @brief TODO: finish docs.
  /// @param sourceFilename
  /// @param sourceCodeLine
  /// @param sourceCodeColumn
  /// @return
  static ref<CodeLocation>
  create(const KValue *source, const std::string &sourceFilename,
         uint64_t sourceCodeLine,
         std::optional<uint64_t> sourceCodeColumn = std::nullopt) {
    return new CodeLocation(Path::PathIndex{0, 0}, source, sourceFilename,
                            sourceCodeLine, sourceCodeColumn);
  }

  /// @brief Converts code location info to SARIFs representation
  /// of location.
  /// @param location location info in source code.
  /// @return SARIFs representation of location.
  PhysicalLocationJson serialize() const;
};

}; // namespace klee

#endif
