#pragma once

namespace CounterExample
{
   // Minimal state object used by the Counter Method example.
   class CounterModel
   {
    public:
      [[nodiscard]] int Value() const noexcept;
      void Add(int amount) noexcept;
      void Reset() noexcept;

    private:
      // Current counter value.
      int m_value{0};
   };
} // namespace CounterExample
