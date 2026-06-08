#pragma once

namespace Method
{
   // Semantic version for a Method declaration.
   struct Version
   {
      // Major version, incremented for breaking changes.
      int major_version{0};

      // Minor version, incremented for compatible changes.
      int minor_version{0};

      bool operator==(const Version &) const = default;
   };
} // namespace Method
