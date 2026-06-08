#pragma once

#include <string>

#include "Method/Declaration.h"

namespace Method
{
   // Runtime invocation of a declared Method.
   class Invocation
   {
    public:
      Invocation(Declaration declaration, std::string description);
      virtual ~Invocation() = default;

      Invocation(const Invocation &) = default;
      Invocation(Invocation &&) noexcept = default;
      Invocation & operator=(const Invocation &) = default;
      Invocation & operator=(Invocation &&) noexcept = default;

      [[nodiscard]] const Declaration & DeclarationMetadata() const noexcept;
      [[nodiscard]] const std::string & Description() const noexcept;

      virtual void Execute() = 0;
      virtual void Undo();

    private:
      // Declaration associated with this specific invocation.
      Declaration m_declaration;

      // Human-friendly description of this invocation.
      std::string m_description;
   };
} // namespace Method
