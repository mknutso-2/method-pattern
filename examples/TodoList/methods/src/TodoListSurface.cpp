#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Method/Kind.h"
#include "TodoListCore/TodoItem.h"
#include "TodoListCore/TodoListModel.h"
#include "TodoListMethods/TodoListSurface.h"

static Method::Json EmptyObjectSchema()
{
   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Json TodoItemSchema()
{
   Method::Json id_schema = Method::Json::object();
   id_schema["type"] = "integer";

   Method::Json title_schema = Method::Json::object();
   title_schema["type"] = "string";

   Method::Json completed_schema = Method::Json::object();
   completed_schema["type"] = "boolean";

   Method::Json properties = Method::Json::object();
   properties["id"] = std::move(id_schema);
   properties["title"] = std::move(title_schema);
   properties["completed"] = std::move(completed_schema);

   Method::Json required = Method::Json::array();
   required.push_back("id");
   required.push_back("title");
   required.push_back("completed");

   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["properties"] = std::move(properties);
   schema["required"] = std::move(required);
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Json TodoListOutputSchema()
{
   Method::Json schema = Method::Json::object();
   schema["type"] = "array";
   schema["items"] = TodoItemSchema();
   return schema;
}

static Method::Json AddTodoInputSchema()
{
   Method::Json title_schema = Method::Json::object();
   title_schema["type"] = "string";
   title_schema["minLength"] = 1;

   Method::Json properties = Method::Json::object();
   properties["title"] = std::move(title_schema);

   Method::Json required = Method::Json::array();
   required.push_back("title");

   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["properties"] = std::move(properties);
   schema["required"] = std::move(required);
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Json CompleteTodoInputSchema()
{
   Method::Json id_schema = Method::Json::object();
   id_schema["type"] = "integer";

   Method::Json properties = Method::Json::object();
   properties["id"] = std::move(id_schema);

   Method::Json required = Method::Json::array();
   required.push_back("id");

   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["properties"] = std::move(properties);
   schema["required"] = std::move(required);
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Json ClearCompletedOutputSchema()
{
   Method::Json count_schema = Method::Json::object();
   count_schema["type"] = "integer";

   Method::Json properties = Method::Json::object();
   properties["removedCount"] = std::move(count_schema);

   Method::Json required = Method::Json::array();
   required.push_back("removedCount");

   Method::Json schema = Method::Json::object();
   schema["type"] = "object";
   schema["properties"] = std::move(properties);
   schema["required"] = std::move(required);
   schema["additionalProperties"] = false;
   return schema;
}

static Method::Declaration ListTodosDeclaration()
{
   return Method::Declaration("todo.list", "List TODO Items", "Return every TODO item currently stored by the model.",
      Method::Kind::Query, Method::Version{1, 0}, EmptyObjectSchema(), TodoListOutputSchema());
}

static Method::Declaration AddTodoDeclaration()
{
   return Method::Declaration("todo.add", "Add TODO Item",
      "Add a new TODO item. The invocation can be undone by removing the created item.", Method::Kind::UndoableCommand,
      Method::Version{1, 0}, AddTodoInputSchema(), TodoItemSchema());
}

static Method::Declaration CompleteTodoDeclaration()
{
   return Method::Declaration("todo.complete", "Complete TODO Item",
      "Mark a TODO item complete. The invocation can be undone by restoring the previous completion state.",
      Method::Kind::UndoableCommand, Method::Version{1, 0}, CompleteTodoInputSchema(), TodoItemSchema());
}

static Method::Declaration ClearCompletedDeclaration()
{
   return Method::Declaration("todo.clearCompleted", "Clear Completed TODO Items",
      "Remove completed TODO items. This example treats bulk removal as non-undoable.",
      Method::Kind::NonUndoableCommand, Method::Version{1, 0}, EmptyObjectSchema(), ClearCompletedOutputSchema());
}

static std::vector<Method::Declaration> ListTodoDeclarations()
{
   return {ListTodosDeclaration(), AddTodoDeclaration(), CompleteTodoDeclaration(), ClearCompletedDeclaration()};
}

static Method::Json SerializeItem(const TodoListCore::TodoItem & item)
{
   Method::Json serialized_item = Method::Json::object();
   serialized_item["id"] = item.id;
   serialized_item["title"] = item.title;
   serialized_item["completed"] = item.completed;
   return serialized_item;
}

static Method::Json SerializeItems(const std::vector<TodoListCore::TodoItem> & items)
{
   Method::Json serialized_items = Method::Json::array();
   for (const auto & item : items)
   {
      serialized_items.push_back(SerializeItem(item));
   }
   return serialized_items;
}

static std::string RequiredString(const Method::Json & input, std::string_view field_name)
{
   const std::string key{field_name};
   if (!input.contains(key) || !input.at(key).is_string())
   {
      throw std::invalid_argument{"Method input is missing a required string field."};
   }

   auto value = input.at(key).get<std::string>();
   if (value.empty())
   {
      throw std::invalid_argument{"String Method input fields must not be empty."};
   }
   return value;
}

static int RequiredInteger(const Method::Json & input, std::string_view field_name)
{
   const std::string key{field_name};
   if (!input.contains(key) || !input.at(key).is_number_integer())
   {
      throw std::invalid_argument{"Method input is missing a required integer field."};
   }

   return input.at(key).get<int>();
}

namespace
{
   class ListTodosInvocation final : public Method::Invocation
   {
    public:
      ListTodosInvocation(Method::Declaration declaration, TodoListCore::TodoListModel & model);
      void Execute() override;
      [[nodiscard]] const Method::Json & Output() const noexcept;

    private:
      // Model read by this invocation.
      TodoListCore::TodoListModel * m_model;

      // JSON array generated during Execute().
      Method::Json m_output{Method::Json::array()};
   };

   class AddTodoInvocation final : public Method::Invocation
   {
    public:
      AddTodoInvocation(Method::Declaration declaration, TodoListCore::TodoListModel & model, std::string title);
      void Execute() override;
      void Undo() override;
      [[nodiscard]] const Method::Json & Output() const noexcept;

    private:
      // Model changed by this invocation.
      TodoListCore::TodoListModel * m_model;

      // Title supplied for the new TODO item.
      std::string m_title;

      // Identifier created during Execute().
      int m_created_id{0};

      // JSON object generated during Execute().
      Method::Json m_output{Method::Json::object()};
   };

   class CompleteTodoInvocation final : public Method::Invocation
   {
    public:
      CompleteTodoInvocation(Method::Declaration declaration, TodoListCore::TodoListModel & model, int id);
      void Execute() override;
      void Undo() override;
      [[nodiscard]] const Method::Json & Output() const noexcept;

    private:
      // Model changed by this invocation.
      TodoListCore::TodoListModel * m_model;

      // TODO item identifier supplied by the caller.
      int m_id;

      // Completion state observed before Execute().
      bool m_previous_completed{false};

      // Whether Execute() has been called.
      bool m_executed{false};

      // JSON object generated during Execute().
      Method::Json m_output{Method::Json::object()};
   };

   class ClearCompletedInvocation final : public Method::Invocation
   {
    public:
      ClearCompletedInvocation(Method::Declaration declaration, TodoListCore::TodoListModel & model);
      void Execute() override;
      [[nodiscard]] const Method::Json & Output() const noexcept;

    private:
      // Model changed by this invocation.
      TodoListCore::TodoListModel * m_model;

      // JSON object generated during Execute().
      Method::Json m_output{Method::Json::object()};
   };
} // namespace

ListTodosInvocation::ListTodosInvocation(Method::Declaration declaration, TodoListCore::TodoListModel & model)
   : Method::Invocation(std::move(declaration), "List TODO items"),
     m_model(&model)
{
}

void ListTodosInvocation::Execute() { m_output = SerializeItems(m_model->List()); }

const Method::Json & ListTodosInvocation::Output() const noexcept { return m_output; }

AddTodoInvocation::AddTodoInvocation(
   Method::Declaration declaration, TodoListCore::TodoListModel & model, std::string title)
   : Method::Invocation(std::move(declaration), "Add TODO item"),
     m_model(&model),
     m_title(std::move(title))
{
}

void AddTodoInvocation::Execute()
{
   const auto item = m_model->Add(m_title);
   m_created_id = item.id;
   m_output = SerializeItem(item);
}

void AddTodoInvocation::Undo()
{
   if (m_created_id == 0)
   {
      throw std::logic_error{"Cannot undo an add invocation before Execute()."};
   }

   static_cast<void>(m_model->Remove(m_created_id));
   m_created_id = 0;
}

const Method::Json & AddTodoInvocation::Output() const noexcept { return m_output; }

CompleteTodoInvocation::CompleteTodoInvocation(
   Method::Declaration declaration, TodoListCore::TodoListModel & model, int id)
   : Method::Invocation(std::move(declaration), "Complete TODO item"),
     m_model(&model),
     m_id(id)
{
}

void CompleteTodoInvocation::Execute()
{
   const auto previous_item = m_model->Item(m_id);
   m_previous_completed = previous_item.completed;
   m_output = SerializeItem(m_model->SetCompleted(m_id, true));
   m_executed = true;
}

void CompleteTodoInvocation::Undo()
{
   if (!m_executed)
   {
      throw std::logic_error{"Cannot undo a complete invocation before Execute()."};
   }

   static_cast<void>(m_model->SetCompleted(m_id, m_previous_completed));
   m_executed = false;
}

const Method::Json & CompleteTodoInvocation::Output() const noexcept { return m_output; }

ClearCompletedInvocation::ClearCompletedInvocation(Method::Declaration declaration, TodoListCore::TodoListModel & model)
   : Method::Invocation(std::move(declaration), "Clear completed TODO items"),
     m_model(&model)
{
}

void ClearCompletedInvocation::Execute()
{
   Method::Json output = Method::Json::object();
   output["removedCount"] = m_model->ClearCompleted();
   m_output = std::move(output);
}

const Method::Json & ClearCompletedInvocation::Output() const noexcept { return m_output; }

static std::unique_ptr<Method::Invocation> CreateTodoInvocation(
   TodoListCore::TodoListModel & model, std::string_view method_id, const Method::Json & input)
{
   if (method_id == "todo.list")
   {
      return std::make_unique<ListTodosInvocation>(ListTodosDeclaration(), model);
   }
   if (method_id == "todo.add")
   {
      return std::make_unique<AddTodoInvocation>(AddTodoDeclaration(), model, RequiredString(input, "title"));
   }
   if (method_id == "todo.complete")
   {
      return std::make_unique<CompleteTodoInvocation>(CompleteTodoDeclaration(), model, RequiredInteger(input, "id"));
   }
   if (method_id == "todo.clearCompleted")
   {
      return std::make_unique<ClearCompletedInvocation>(ClearCompletedDeclaration(), model);
   }

   throw std::invalid_argument{"Unknown TODO Method."};
}

namespace TodoListMethods
{
   class TodoListSurfaceImpl
   {
    public:
      TodoListSurfaceImpl() = default;
      ~TodoListSurfaceImpl() = default;

      TodoListSurfaceImpl(TodoListSurfaceImpl && other) noexcept = default;
      TodoListSurfaceImpl & operator=(TodoListSurfaceImpl && other) noexcept = default;
      TodoListSurfaceImpl(const TodoListSurfaceImpl & other) = delete;
      TodoListSurfaceImpl & operator=(const TodoListSurfaceImpl & other) = delete;

      [[nodiscard]] std::vector<Method::Declaration> Declarations() const;
      [[nodiscard]] std::vector<Method::Definition> Definitions(const Method::Json & input);
      [[nodiscard]] Method::Json Invoke(std::string_view method_id, const Method::Json & input);
      bool UndoLast();

    private:
      // Core model hidden behind the Method-facing API.
      TodoListCore::TodoListModel m_model;

      // Most recent undoable invocation, retained so a surface can trigger undo.
      std::unique_ptr<Method::Invocation> m_last_undoable;
   };
} // namespace TodoListMethods

std::vector<Method::Declaration> TodoListMethods::TodoListSurfaceImpl::Declarations() const
{
   return ListTodoDeclarations();
}

std::vector<Method::Definition> TodoListMethods::TodoListSurfaceImpl::Definitions(const Method::Json & input)
{
   auto declarations = Declarations();
   std::vector<Method::Definition> definitions;
   definitions.reserve(declarations.size());

   for (auto declaration : declarations)
   {
      const auto method_id = declaration.Id();
      definitions.emplace_back(std::move(declaration),
         [model = &m_model, method_id, input]() -> std::unique_ptr<Method::Invocation>
         { return CreateTodoInvocation(*model, method_id, input); });
   }

   return definitions;
}

Method::Json TodoListMethods::TodoListSurfaceImpl::Invoke(std::string_view method_id, const Method::Json & input)
{
   if (method_id == "todo.list")
   {
      ListTodosInvocation invocation(ListTodosDeclaration(), m_model);
      invocation.Execute();
      return invocation.Output();
   }
   if (method_id == "todo.add")
   {
      auto invocation =
         std::make_unique<AddTodoInvocation>(AddTodoDeclaration(), m_model, RequiredString(input, "title"));
      invocation->Execute();
      auto output = invocation->Output();
      m_last_undoable = std::move(invocation);
      return output;
   }
   if (method_id == "todo.complete")
   {
      auto invocation =
         std::make_unique<CompleteTodoInvocation>(CompleteTodoDeclaration(), m_model, RequiredInteger(input, "id"));
      invocation->Execute();
      auto output = invocation->Output();
      m_last_undoable = std::move(invocation);
      return output;
   }
   if (method_id == "todo.clearCompleted")
   {
      ClearCompletedInvocation invocation(ClearCompletedDeclaration(), m_model);
      invocation.Execute();
      m_last_undoable.reset();
      return invocation.Output();
   }

   throw std::invalid_argument{"Unknown TODO Method."};
}

bool TodoListMethods::TodoListSurfaceImpl::UndoLast()
{
   if (m_last_undoable == nullptr)
   {
      return false;
   }

   m_last_undoable->Undo();
   m_last_undoable.reset();
   return true;
}

TodoListMethods::TodoListSurface::TodoListSurface() : m_impl(std::make_unique<TodoListSurfaceImpl>()) {}

TodoListMethods::TodoListSurface::~TodoListSurface() = default;

TodoListMethods::TodoListSurface::TodoListSurface(TodoListSurface && other) noexcept = default;

TodoListMethods::TodoListSurface & TodoListMethods::TodoListSurface::operator=(
   TodoListSurface && other) noexcept = default;

std::vector<Method::Declaration> TodoListMethods::TodoListSurface::Declarations() const
{
   return m_impl->Declarations();
}

std::vector<Method::Definition> TodoListMethods::TodoListSurface::Definitions(const Method::Json & input)
{
   return m_impl->Definitions(input);
}

Method::Json TodoListMethods::TodoListSurface::Invoke(std::string_view method_id, const Method::Json & input)
{
   return m_impl->Invoke(method_id, input);
}

bool TodoListMethods::TodoListSurface::UndoLast() { return m_impl->UndoLast(); }
