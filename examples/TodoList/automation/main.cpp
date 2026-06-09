#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Method/Json.h"
#include "TodoListMethods/TodoListSurface.h"

static std::string ReadStandardInput()
{
   std::ostringstream buffer;
   buffer << std::cin.rdbuf();
   return buffer.str();
}

static Method::Json InvokeRequest(TodoListMethods::TodoListSurface & surface, const Method::Json & request)
{
   if (!request.contains("method") || !request.at("method").is_string())
   {
      throw std::invalid_argument{"Automation request requires a string method field."};
   }

   Method::Json parameters = Method::Json::object();
   if (request.contains("parameters"))
   {
      parameters = request.at("parameters");
   }

   return surface.Invoke(request.at("method").get<std::string>(), parameters);
}

static Method::Json InvokeRequests(TodoListMethods::TodoListSurface & surface, const Method::Json & request)
{
   if (request.is_array())
   {
      Method::Json results = Method::Json::array();
      for (const auto & entry : request)
      {
         results.push_back(InvokeRequest(surface, entry));
      }
      return results;
   }

   return InvokeRequest(surface, request);
}

int main(int argc, char ** argv)
{
   try
   {
      const auto input = argc > 1 ? std::string{argv[1]} : ReadStandardInput();
      if (input.empty())
      {
         throw std::invalid_argument{"Provide an automation request as JSON on argv[1] or stdin."};
      }

      TodoListMethods::TodoListSurface surface;
      const auto request = Method::Json::parse(input);
      std::cout << InvokeRequests(surface, request).dump(3) << '\n';
      return 0;
   }
   catch (const std::exception & error)
   {
      std::cerr << error.what() << '\n';
      return 1;
   }
}
