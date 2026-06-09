#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "CounterMethods.h"
#include "CounterModel.h"
#include "Method/Declaration.h"
#include "Method/Definition.h"
#include "Method/Json.h"
#include "Method/Kind.h"
#include "TodoListMethods/TodoListSurface.h"

static const Method::Declaration & FindDeclaration(
   const std::vector<Method::Declaration> & declarations, std::string_view method_id)
{
   for (const auto & declaration : declarations)
   {
      if (declaration.Id() == std::string{method_id})
      {
         return declaration;
      }
   }

   throw std::invalid_argument{"Unknown Method declaration."};
}

static const Method::Definition & FindDefinition(
   const std::vector<Method::Definition> & definitions, std::string_view method_id)
{
   for (const auto & definition : definitions)
   {
      if (definition.DeclarationMetadata().Id() == std::string{method_id})
      {
         return definition;
      }
   }

   throw std::invalid_argument{"Unknown Method definition."};
}

static Method::Json InputWithTitle(std::string title)
{
   Method::Json input = Method::Json::object();
   input["title"] = std::move(title);
   return input;
}

static Method::Json InputWithId(int id)
{
   Method::Json input = Method::Json::object();
   input["id"] = id;
   return input;
}

namespace
{
   TEST_CASE("counter example exposes query command and undo metadata")
   {
      const auto declarations = CounterExample::ListCounterDeclarations();

      REQUIRE(declarations.size() == 3U);
      REQUIRE(FindDeclaration(declarations, "counter.value").MethodKind() == Method::Kind::Query);
      REQUIRE(FindDeclaration(declarations, "counter.add").MethodKind() == Method::Kind::UndoableCommand);
      REQUIRE(FindDeclaration(declarations, "counter.reset").MethodKind() == Method::Kind::NonUndoableCommand);
      REQUIRE(FindDeclaration(declarations, "counter.add").InputSchema().at("required").at(0) == "amount");
   }

   TEST_CASE("counter example definitions execute and undo against the model")
   {
      CounterExample::CounterModel counter;
      const auto definitions = CounterExample::CreateCounterDefinitions(counter, 7);

      auto add_invocation = FindDefinition(definitions, "counter.add").CreateInvocation();
      add_invocation->Execute();

      REQUIRE(counter.Value() == 7);

      add_invocation->Undo();

      REQUIRE(counter.Value() == 0);

      counter.Add(12);
      auto reset_invocation = FindDefinition(definitions, "counter.reset").CreateInvocation();
      reset_invocation->Execute();

      REQUIRE(counter.Value() == 0);
   }

   TEST_CASE("todo example exposes a Method surface without exposing the core model")
   {
      TodoListMethods::TodoListSurface surface;
      const auto declarations = surface.Declarations();

      REQUIRE(declarations.size() == 4U);
      REQUIRE(FindDeclaration(declarations, "todo.list").MethodKind() == Method::Kind::Query);
      REQUIRE(FindDeclaration(declarations, "todo.add").MethodKind() == Method::Kind::UndoableCommand);
      REQUIRE(FindDeclaration(declarations, "todo.complete").MethodKind() == Method::Kind::UndoableCommand);
      REQUIRE(FindDeclaration(declarations, "todo.clearCompleted").MethodKind() == Method::Kind::NonUndoableCommand);
      REQUIRE(FindDeclaration(declarations, "todo.add").InputSchema().at("required").at(0) == "title");
   }

   TEST_CASE("todo example supports add list complete and undo through the surface")
   {
      TodoListMethods::TodoListSurface surface;

      const auto added_item = surface.Invoke("todo.add", InputWithTitle("Write tests"));
      REQUIRE(added_item.at("id") == 1);
      REQUIRE(added_item.at("title") == "Write tests");
      REQUIRE(added_item.at("completed") == false);

      auto listed_items = surface.Invoke("todo.list", Method::Json::object());
      REQUIRE(listed_items.size() == 1U);
      REQUIRE(listed_items.at(0).at("completed") == false);

      const auto completed_item = surface.Invoke("todo.complete", InputWithId(1));
      REQUIRE(completed_item.at("completed") == true);

      listed_items = surface.Invoke("todo.list", Method::Json::object());
      REQUIRE(listed_items.at(0).at("completed") == true);

      REQUIRE(surface.UndoLast());

      listed_items = surface.Invoke("todo.list", Method::Json::object());
      REQUIRE(listed_items.at(0).at("completed") == false);
      REQUIRE_FALSE(surface.UndoLast());
   }

   TEST_CASE("todo example clear completed removes only completed items")
   {
      TodoListMethods::TodoListSurface surface;
      static_cast<void>(surface.Invoke("todo.add", InputWithTitle("Write docs")));
      static_cast<void>(surface.Invoke("todo.add", InputWithTitle("Run tests")));
      static_cast<void>(surface.Invoke("todo.complete", InputWithId(1)));

      const auto clear_output = surface.Invoke("todo.clearCompleted", Method::Json::object());
      REQUIRE(clear_output.at("removedCount") == 1);

      const auto listed_items = surface.Invoke("todo.list", Method::Json::object());
      REQUIRE(listed_items.size() == 1U);
      REQUIRE(listed_items.at(0).at("id") == 2);
      REQUIRE(listed_items.at(0).at("title") == "Run tests");
      REQUIRE(listed_items.at(0).at("completed") == false);
   }

   TEST_CASE("todo example definitions create invocations bound to JSON input")
   {
      TodoListMethods::TodoListSurface surface;
      const auto definitions = surface.Definitions(InputWithTitle("Call from a Method definition"));

      auto add_invocation = FindDefinition(definitions, "todo.add").CreateInvocation();
      add_invocation->Execute();

      const auto listed_items = surface.Invoke("todo.list", Method::Json::object());
      REQUIRE(listed_items.size() == 1U);
      REQUIRE(listed_items.at(0).at("title") == "Call from a Method definition");
   }
} // namespace
