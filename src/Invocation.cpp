#include <stdexcept>
#include <utility>

#include "Method/Invocation.h"

Method::Invocation::Invocation(Declaration declaration, std::string description)
   : m_declaration{std::move(declaration)},
     m_description{std::move(description)}
{
}

const Method::Declaration & Method::Invocation::DeclarationMetadata() const noexcept { return m_declaration; }

const std::string & Method::Invocation::Description() const noexcept { return m_description; }

void Method::Invocation::Undo() { throw std::logic_error{"Method invocation does not support undo"}; }
