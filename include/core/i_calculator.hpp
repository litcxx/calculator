#ifndef I_CALCULATOR_HPP_
#define I_CALCULATOR_HPP_

#include "utils/config.hpp"

namespace calculator
{
class ICalculator // NOLINT
{
  public:
    virtual ~ICalculator() = default;
    virtual Task calculate(const CalculationRequest& request) = 0;
};
} // namespace calculator

#endif
