#ifndef I_PARSER_HPP_
#define I_PARSER_HPP_

#include "utils/config.hpp"

#include <string_view>

namespace calculator
{
class IParser // NOLINT
{
  public:
    virtual ~IParser() = default;
    virtual void validateInput(int argc, char** argv) = 0;
    virtual CalculationRequest parse(std::string_view line) = 0;
};
} // namespace calculator

#endif
