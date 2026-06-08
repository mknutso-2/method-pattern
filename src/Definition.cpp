#include <stdexcept>
#include <utility>

#include "Method/Definition.h"

Method::Definition::Definition(Declaration declaration, InvocationFactory invocation_factory)
   : m_declaration{std::move(declaration)},
     m_invocation_factory{std::move(invocation_factory)}
{
}

const Method::Declaration & Method::Definition::DeclarationMetadata() const noexcept { return m_declaration; }

std::unique_ptr<Method::Invocation> Method::Definition::CreateInvocation() const
{
   if (!m_invocation_factory)
   {
      throw std::logic_error{"Method definition has no invocation factory"};
   }

   auto invocation = m_invocation_factory();
   if (invocation == nullptr)
   {
      throw std::logic_error{"Method invocation factory returned null"};
   }
   return invocation;
}
