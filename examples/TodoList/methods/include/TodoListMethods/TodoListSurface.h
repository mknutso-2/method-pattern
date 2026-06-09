#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Method/Declaration.h"
#include "Method/Definition.h"
#include "Method/Json.h"

namespace TodoListMethods
{
   class TodoListSurfaceImpl;

   // Method-facing TODO list API exposed to surfaces such as CLI and automation adapters.
   class TodoListSurface
   {
    public:
      TodoListSurface();
      ~TodoListSurface();

      TodoListSurface(TodoListSurface && other) noexcept;
      TodoListSurface & operator=(TodoListSurface && other) noexcept;
      TodoListSurface(const TodoListSurface & other) = delete;
      TodoListSurface & operator=(const TodoListSurface & other) = delete;

      [[nodiscard]] std::vector<Method::Declaration> Declarations() const;
      [[nodiscard]] std::vector<Method::Definition> Definitions(const Method::Json & input = Method::Json::object());
      [[nodiscard]] Method::Json Invoke(std::string_view method_id, const Method::Json & input);
      bool UndoLast();

    private:
      // Private implementation that owns the core model.
      std::unique_ptr<TodoListSurfaceImpl> m_impl;
   };
} // namespace TodoListMethods
