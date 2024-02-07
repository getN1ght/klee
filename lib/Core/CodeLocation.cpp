#include "klee/Core/CodeLocation.h"

#include "klee/Module/LocationInfo.h"
#include "klee/Module/SarifReport.h"

#include <optional>
#include <utility>

using namespace klee;

/// @brief Converts location info to SARIFs representation
/// of location.
/// @param location location info in source code.
/// @return SARIFs representation of location.
static PhysicalLocationJson from(const LocationInfo &location) {
  // Clang-format does not keep pretty format for JSON.
  // For better readability split creating of JSON in local variables.

  ArtifactLocationJson artifactLocationJson{{location.file}};
  RegionJson regionJson{
      {location.line}, std::nullopt, location.column, std::nullopt};
  PhysicalLocationJson physicalLocationJson{{std::move(artifactLocationJson)},
                                            {std::move(regionJson)}};

  return physicalLocationJson;
}

////////////////////////////////////////////////////////////////

PhysicalLocationJson CodeLocation::serialize() const { return from(location); }
