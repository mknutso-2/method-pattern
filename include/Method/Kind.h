#pragma once

namespace Method
{
   // Describes whether invoking a Method reads data, performs undoable work, or performs non-undoable work.
   enum class Kind
   {
      // Method reads data without changing durable state.
      Query,

      // Method changes durable state and supports undo.
      UndoableCommand,

      // Method changes durable state and does not support undo.
      NonUndoableCommand,
   };
} // namespace Method
