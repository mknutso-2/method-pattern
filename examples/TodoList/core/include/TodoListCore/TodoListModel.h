#pragma once

#include <string>
#include <vector>

#include "TodoListCore/TodoItem.h"

namespace TodoListCore
{
   // Core TODO list model with no Method, CLI, or automation dependencies.
   class TodoListModel
   {
    public:
      TodoItem Add(std::string title);
      [[nodiscard]] TodoItem Item(int id) const;
      [[nodiscard]] bool Remove(int id);
      TodoItem SetCompleted(int id, bool completed);
      [[nodiscard]] std::vector<TodoItem> List() const;
      int ClearCompleted();

    private:
      [[nodiscard]] std::vector<TodoItem>::iterator FindItem(int id);
      [[nodiscard]] std::vector<TodoItem>::const_iterator FindItem(int id) const;

      // Next identifier assigned to a new item.
      int m_next_id{1};

      // Items currently stored by the model.
      std::vector<TodoItem> m_items;
   };
} // namespace TodoListCore
