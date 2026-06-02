#pragma once

#include "config.hpp"

#include <string_view>

namespace calculator
{
class Parser
{
  public:
    static CalculationRequest Parse(std::string_view line);

  private:
    static Operation toOperation(std::string_view operation);
};
} // namespace calculator
