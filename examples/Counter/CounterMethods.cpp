#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "CounterMethods.h"
#include "Method/Kind.h"

static Method::Json EmptyObjectSchema()
{
   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Json CounterValueSchema()
{
   Method::Json value_schema = Method::Json::object();
   value_schema["type"] = "integer";

   Method::Json properties = Method::Json::object();
   properties["value"] = std::move(value_schema);

   Method::Json required = Method::Json::array();
   required.push_back("value");

   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["properties"] = std::move(properties);
   schema["required"] = std::move(required);
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Json AddInputSchema()
{
   Method::Json amount_schema = Method::Json::object();
   amount_schema["type"] = "integer";

   Method::Json properties = Method::Json::object();
   properties["amount"] = std::move(amount_schema);

   Method::Json required = Method::Json::array();
   required.push_back("amount");

   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["properties"] = std::move(properties);
   schema["required"] = std::move(required);
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Declaration CounterValueDeclaration()
{
   return Method::Declaration("counter.value", "Counter Value", "Read the current counter value.", Method::Kind::Query,
      Method::Version{1, 0}, EmptyObjectSchema(), CounterValueSchema());
}

static Method::Declaration CounterAddDeclaration()
{
   return Method::Declaration("counter.add", "Add",
      "Add an integer amount to the counter. The invocation can be undone.", Method::Kind::UndoableCommand,
      Method::Version{1, 0}, AddInputSchema(), CounterValueSchema());
}

static Method::Declaration CounterResetDeclaration()
{
   return Method::Declaration("counter.reset", "Reset",
      "Reset the counter to zero. This example treats reset as non-undoable.", Method::Kind::NonUndoableCommand,
      Method::Version{1, 0}, EmptyObjectSchema(), CounterValueSchema());
}

std::vector<Method::Declaration> CounterExample::ListCounterDeclarations()
{
   return {CounterValueDeclaration(), CounterAddDeclaration(), CounterResetDeclaration()};
}

namespace
{
   class CounterValueInvocation final : public Method::Invocation
   {
    public:
      CounterValueInvocation(Method::Declaration declaration, CounterExample::CounterModel & model);
      void Execute() override;

    private:
      // Model read by this invocation.
      CounterExample::CounterModel * m_model;
   };

   class CounterAddInvocation final : public Method::Invocation
   {
    public:
      CounterAddInvocation(Method::Declaration declaration, CounterExample::CounterModel & model, int amount);
      void Execute() override;
      void Undo() override;

    private:
      // Model changed by this invocation.
      CounterExample::CounterModel * m_model;

      // Amount added during Execute() and removed during Undo().
      int m_amount;

      // Whether Execute() has been called.
      bool m_executed{false};
   };

   class CounterResetInvocation final : public Method::Invocation
   {
    public:
      CounterResetInvocation(Method::Declaration declaration, CounterExample::CounterModel & model);
      void Execute() override;

    private:
      // Model changed by this invocation.
      CounterExample::CounterModel * m_model;
   };
} // namespace

CounterValueInvocation::CounterValueInvocation(Method::Declaration declaration, CounterExample::CounterModel & model)
   : Method::Invocation(std::move(declaration), "Read the counter value"),
     m_model(&model)
{
}

void CounterValueInvocation::Execute() { static_cast<void>(m_model->Value()); }

CounterAddInvocation::CounterAddInvocation(
   Method::Declaration declaration, CounterExample::CounterModel & model, int amount)
   : Method::Invocation(std::move(declaration), "Add to the counter"),
     m_model(&model),
     m_amount(amount)
{
}

void CounterAddInvocation::Execute()
{
   m_model->Add(m_amount);
   m_executed = true;
}

void CounterAddInvocation::Undo()
{
   if (!m_executed)
   {
      throw std::logic_error{"Cannot undo a counter add invocation before Execute()."};
   }

   m_model->Add(-m_amount);
   m_executed = false;
}

CounterResetInvocation::CounterResetInvocation(Method::Declaration declaration, CounterExample::CounterModel & model)
   : Method::Invocation(std::move(declaration), "Reset the counter"),
     m_model(&model)
{
}

void CounterResetInvocation::Execute() { m_model->Reset(); }

std::vector<Method::Definition> CounterExample::CreateCounterDefinitions(CounterModel & model, int add_amount)
{
   auto value_declaration = CounterValueDeclaration();
   auto add_declaration = CounterAddDeclaration();
   auto reset_declaration = CounterResetDeclaration();

   std::vector<Method::Definition> definitions;
   definitions.reserve(3);
   definitions.emplace_back(value_declaration, [&model, value_declaration]() -> std::unique_ptr<Method::Invocation>
      { return std::make_unique<CounterValueInvocation>(value_declaration, model); });
   definitions.emplace_back(add_declaration,
      [&model, add_declaration, add_amount]() -> std::unique_ptr<Method::Invocation>
      { return std::make_unique<CounterAddInvocation>(add_declaration, model, add_amount); });
   definitions.emplace_back(reset_declaration, [&model, reset_declaration]() -> std::unique_ptr<Method::Invocation>
      { return std::make_unique<CounterResetInvocation>(reset_declaration, model); });
   return definitions;
}
