#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include "Method/Declaration.h"
#include "Method/Definition.h"
#include "Method/Invocation.h"
#include "Method/Json.h"
#include "Method/Kind.h"
#include "Method/Version.h"

static Method::Declaration CreateDeclaration(Method::Kind kind = Method::Kind::Query,
   Method::Version version = Method::Version{.major_version = 1, .minor_version = 0})
{
   auto input_schema = Method::Json::object();
   input_schema["type"] = "object";
   input_schema["required"] = Method::Json::array({"query"});

   auto output_schema = Method::Json::object();
   output_schema["type"] = "array";

   return Method::Declaration("recipe.search", "Search Recipes", "Finds recipes that match the requested filters.",
      kind, version, std::move(input_schema), std::move(output_schema));
}

namespace
{
   class CountingInvocation final : public Method::Invocation
   {
    public:
      CountingInvocation(Method::Declaration declaration, std::string description, std::shared_ptr<int> executions);

      void Execute() override;

    private:
      std::shared_ptr<int> m_executions;
   };

   CountingInvocation::CountingInvocation(
      Method::Declaration declaration, std::string description, std::shared_ptr<int> executions)
      : Method::Invocation(std::move(declaration), std::move(description)),
        m_executions{std::move(executions)}
   {
   }

   void CountingInvocation::Execute() { ++(*m_executions); }

   TEST_CASE("declaration exposes method metadata")
   {
      const auto declaration =
         CreateDeclaration(Method::Kind::UndoableCommand, Method::Version{.major_version = 2, .minor_version = 1});

      REQUIRE(declaration.Id() == "recipe.search");
      REQUIRE(declaration.DisplayName() == "Search Recipes");
      REQUIRE(declaration.Description() == "Finds recipes that match the requested filters.");
      REQUIRE(declaration.MethodKind() == Method::Kind::UndoableCommand);
      REQUIRE(declaration.MethodVersion() == Method::Version{.major_version = 2, .minor_version = 1});
      REQUIRE(declaration.InputSchema().at("type") == "object");
      REQUIRE(declaration.OutputSchema().at("type") == "array");
   }

   TEST_CASE("definition creates concrete invocations")
   {
      const auto executions = std::make_shared<int>(0);
      const auto declaration = CreateDeclaration();
      const auto definition = Method::Definition(declaration, [&declaration, &executions]
         { return std::make_unique<CountingInvocation>(declaration, "Search for dinner recipes.", executions); });

      const auto invocation = definition.CreateInvocation();

      REQUIRE(definition.DeclarationMetadata().Id() == declaration.Id());
      REQUIRE(invocation->DeclarationMetadata().Id() == declaration.Id());
      REQUIRE(invocation->Description() == "Search for dinner recipes.");

      invocation->Execute();

      REQUIRE(*executions == 1);
   }

   TEST_CASE("definition rejects missing invocation factory")
   {
      const auto definition = Method::Definition(CreateDeclaration(), nullptr);

      REQUIRE_THROWS_AS(definition.CreateInvocation(), std::logic_error);
   }

   TEST_CASE("invocation undo throws unless overridden")
   {
      const auto executions = std::make_shared<int>(0);
      auto invocation = CountingInvocation(CreateDeclaration(), "Search for pantry recipes.", executions);

      REQUIRE_THROWS_AS(invocation.Undo(), std::logic_error);
   }
} // namespace
