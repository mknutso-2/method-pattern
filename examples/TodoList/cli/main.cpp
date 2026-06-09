#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Method/Json.h"
#include "TodoListMethods/TodoListSurface.h"

static void PrintJson(const Method::Json & value) { std::cout << value.dump(3) << '\n'; }

static int ParseInteger(std::string_view text)
{
   std::size_t processed_characters{0};
   const auto value = std::stoi(std::string{text}, &processed_characters);
   if (processed_characters != text.size())
   {
      throw std::invalid_argument{"Expected an integer argument."};
   }

   return value;
}

static void RunDemo(TodoListMethods::TodoListSurface & surface)
{
   Method::Json add_first_input = Method::Json::object();
   add_first_input["title"] = "Write Method documentation";
   PrintJson(surface.Invoke("todo.add", add_first_input));

   Method::Json add_second_input = Method::Json::object();
   add_second_input["title"] = "Wire automation adapter";
   PrintJson(surface.Invoke("todo.add", add_second_input));

   Method::Json complete_input = Method::Json::object();
   complete_input["id"] = 1;
   PrintJson(surface.Invoke("todo.complete", complete_input));

   PrintJson(surface.Invoke("todo.list", Method::Json::object()));
}

int main(int argc, char ** argv)
{
   try
   {
      TodoListMethods::TodoListSurface surface;
      const auto argument_count = static_cast<std::size_t>(argc);
      if (argument_count == 1)
      {
         RunDemo(surface);
         return 0;
      }

      std::size_t argument_index{1};
      while (argument_index < argument_count)
      {
         const std::string_view command{argv[argument_index]};
         ++argument_index;

         if (command == "add")
         {
            if (argument_index >= argument_count)
            {
               throw std::invalid_argument{"add requires a title argument."};
            }

            Method::Json input = Method::Json::object();
            input["title"] = argv[argument_index];
            ++argument_index;
            PrintJson(surface.Invoke("todo.add", input));
         }
         else if (command == "complete")
         {
            if (argument_index >= argument_count)
            {
               throw std::invalid_argument{"complete requires an id argument."};
            }

            Method::Json input = Method::Json::object();
            input["id"] = ParseInteger(argv[argument_index]);
            ++argument_index;
            PrintJson(surface.Invoke("todo.complete", input));
         }
         else if (command == "clear-completed")
         {
            PrintJson(surface.Invoke("todo.clearCompleted", Method::Json::object()));
         }
         else if (command == "list")
         {
            PrintJson(surface.Invoke("todo.list", Method::Json::object()));
         }
         else if (command == "undo")
         {
            Method::Json output = Method::Json::object();
            output["undone"] = surface.UndoLast();
            PrintJson(output);
         }
         else
         {
            throw std::invalid_argument{"Unknown TODO CLI command."};
         }
      }

      return 0;
   }
   catch (const std::exception & error)
   {
      std::cerr << error.what() << '\n';
      return 1;
   }
}
