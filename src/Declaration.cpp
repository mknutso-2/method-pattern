#include <utility>

#include "Method/Declaration.h"

Method::Declaration::Declaration(std::string id,
   std::string display_name,
   std::string description,
   Kind kind,
   Version version,
   Json input_schema,
   Json output_schema)
   : m_id(std::move(id)),
     m_display_name(std::move(display_name)),
     m_description(std::move(description)),
     m_kind(kind),
     m_version(version),
     m_input_schema(std::move(input_schema)),
     m_output_schema(std::move(output_schema))
{
}

const std::string & Method::Declaration::Id() const noexcept { return m_id; }

const std::string & Method::Declaration::DisplayName() const noexcept { return m_display_name; }

const std::string & Method::Declaration::Description() const noexcept { return m_description; }

Method::Kind Method::Declaration::MethodKind() const noexcept { return m_kind; }

const Method::Version & Method::Declaration::MethodVersion() const noexcept { return m_version; }

const Method::Json & Method::Declaration::InputSchema() const noexcept { return m_input_schema; }

const Method::Json & Method::Declaration::OutputSchema() const noexcept { return m_output_schema; }
