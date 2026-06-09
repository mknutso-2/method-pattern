#pragma once

#include <string>

namespace TodoListCore
{
   // Domain object stored by the TODO list model.
   struct TodoItem
   {
      // Stable item identifier for the lifetime of a model instance.
      int id{0};

      // User-visible task title.
      std::string title;

      // Whether the task has been completed.
      bool completed{false};
   };
} // namespace TodoListCore
