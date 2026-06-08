#pragma once

#include <string>

#include "Method/Json.h"
#include "Method/Kind.h"
#include "Method/Version.h"

namespace Method
{
   // Declarative metadata for a query or command-style operation.
   class Declaration
   {
    public:
      Declaration(std::string id,
         std::string display_name,
         std::string description,
         Kind kind,
         Version version,
         Json input_schema,
         Json output_schema);

      [[nodiscard]] const std::string & Id() const noexcept;
      [[nodiscard]] const std::string & DisplayName() const noexcept;
      [[nodiscard]] const std::string & Description() const noexcept;
      [[nodiscard]] Kind MethodKind() const noexcept;
      [[nodiscard]] const Version & MethodVersion() const noexcept;
      [[nodiscard]] const Json & InputSchema() const noexcept;
      [[nodiscard]] const Json & OutputSchema() const noexcept;

    private:
      // Stable identifier used by callers to select this Method.
      std::string m_id;

      // Human-friendly name suitable for menus, logs, and documentation.
      std::string m_display_name;

      // Human-friendly explanation of what the Method does.
      std::string m_description;

      // Operation category used by callers to understand side effects and undo support.
      Kind m_kind;

      // Public declaration version.
      Version m_version;

      // JSON schema describing valid invocation input.
      Json m_input_schema;

      // JSON schema describing invocation output.
      Json m_output_schema;
   };
} // namespace Method
