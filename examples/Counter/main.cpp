#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "CounterMethods.h"
#include "CounterModel.h"

static const Method::Definition & FindDefinition(
   const std::vector<Method::Definition> & definitions, std::string_view method_id)
{
   for (const auto & definition : definitions)
   {
      if (definition.DeclarationMetadata().Id() == method_id)
      {
         return definition;
      }
   }

   throw std::invalid_argument{"Unknown Counter Method."};
}

int main()
{
   try
   {
      CounterExample::CounterModel counter;
      const auto declarations = CounterExample::ListCounterDeclarations();

      std::cout << "Counter Methods:\n";
      for (const auto & declaration : declarations)
      {
         std::cout << " - " << declaration.Id() << '\n';
      }

      const auto definitions = CounterExample::CreateCounterDefinitions(counter, 5);
      auto add_invocation = FindDefinition(definitions, "counter.add").CreateInvocation();
      add_invocation->Execute();
      std::cout << "After add: " << counter.Value() << '\n';

      add_invocation->Undo();
      std::cout << "After undo: " << counter.Value() << '\n';

      counter.Add(12);
      auto reset_invocation = FindDefinition(definitions, "counter.reset").CreateInvocation();
      reset_invocation->Execute();
      std::cout << "After reset: " << counter.Value() << '\n';
      return 0;
   }
   catch (const std::exception & error)
   {
      std::cerr << error.what() << '\n';
      return 1;
   }
}
