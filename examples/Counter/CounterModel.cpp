#include "CounterModel.h"

int CounterExample::CounterModel::Value() const noexcept { return m_value; }

void CounterExample::CounterModel::Add(int amount) noexcept { m_value += amount; }

void CounterExample::CounterModel::Reset() noexcept { m_value = 0; }
