#include <stdexcept>
#include <utility>

#include "TodoListCore/TodoListModel.h"

TodoListCore::TodoItem TodoListCore::TodoListModel::Add(std::string title)
{
   TodoItem item{m_next_id, std::move(title), false};
   ++m_next_id;
   m_items.push_back(item);
   return item;
}

TodoListCore::TodoItem TodoListCore::TodoListModel::Item(int id) const
{
   const auto item = FindItem(id);
   if (item == m_items.cend())
   {
      throw std::out_of_range{"TODO item does not exist."};
   }

   return *item;
}

bool TodoListCore::TodoListModel::Remove(int id)
{
   const auto item = FindItem(id);
   if (item == m_items.end())
   {
      return false;
   }

   m_items.erase(item);
   return true;
}

TodoListCore::TodoItem TodoListCore::TodoListModel::SetCompleted(int id, bool completed)
{
   const auto item = FindItem(id);
   if (item == m_items.end())
   {
      throw std::out_of_range{"TODO item does not exist."};
   }

   item->completed = completed;
   return *item;
}

std::vector<TodoListCore::TodoItem> TodoListCore::TodoListModel::List() const { return m_items; }

int TodoListCore::TodoListModel::ClearCompleted()
{
   int removed_count{0};
   auto item = m_items.begin();
   while (item != m_items.end())
   {
      if (item->completed)
      {
         item = m_items.erase(item);
         ++removed_count;
      }
      else
      {
         ++item;
      }
   }

   return removed_count;
}

std::vector<TodoListCore::TodoItem>::iterator TodoListCore::TodoListModel::FindItem(int id)
{
   for (auto item = m_items.begin(); item != m_items.end(); ++item)
   {
      if (item->id == id)
      {
         return item;
      }
   }

   return m_items.end();
}

std::vector<TodoListCore::TodoItem>::const_iterator TodoListCore::TodoListModel::FindItem(int id) const
{
   for (auto item = m_items.cbegin(); item != m_items.cend(); ++item)
   {
      if (item->id == id)
      {
         return item;
      }
   }

   return m_items.cend();
}
