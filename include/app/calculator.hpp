#ifndef CALCULATOR_HPP_
#define CALCULATOR_HPP_

#include "core/i_calculator.hpp"
#include "utils/config.hpp"

namespace calculator
{
class Calculator : public ICalculator
{
  public:
    Task calculate(const CalculationRequest& request) override;
};
} // namespace calculator
#endif
