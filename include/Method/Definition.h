#pragma once

#include <functional>
#include <memory>

#include "Method/Declaration.h"
#include "Method/Invocation.h"

namespace Method
{
   // Executable definition for a declared Method.
   class Definition
   {
    public:
      // Factory that creates a concrete invocation instance for this Method.
      using InvocationFactory = std::function<std::unique_ptr<Invocation>()>;

      Definition(Declaration declaration, InvocationFactory invocation_factory);

      [[nodiscard]] const Declaration & DeclarationMetadata() const noexcept;
      [[nodiscard]] std::unique_ptr<Invocation> CreateInvocation() const;

    private:
      // Public declaration for this Method.
      Declaration m_declaration;

      // Factory used to create concrete invocations.
      InvocationFactory m_invocation_factory;
   };
} // namespace Method
