#pragma once

#include <vector>

#include "CounterModel.h"
#include "Method/Declaration.h"
#include "Method/Definition.h"

namespace CounterExample
{
   std::vector<Method::Declaration> ListCounterDeclarations();
   std::vector<Method::Definition> CreateCounterDefinitions(CounterModel & model, int add_amount);
} // namespace CounterExample
