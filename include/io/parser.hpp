#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "core/i_parser.hpp"
#include "utils/config.hpp"

#include <string_view>

namespace calculator
{
class Parser : public IParser
{
  public:
    void validateInput(int argc, char** argv) override;
    CalculationRequest parse(std::string_view line) override;
};
} // namespace calculator

#endif
