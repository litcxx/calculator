#ifndef PARSER_H_
#define PARSER_H_

#include "config.hpp"

#include <string_view>

namespace calculator
{
class Parser
{
  public:
    static CalculationRequest parse(std::string_view line);

  private:
    static Operation toOperation(std::string_view operation);
};
} // namespace calculator

#endif
