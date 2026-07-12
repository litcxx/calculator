#include "app/calculator.hpp"

#include "litmath/math.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"

namespace calculator
{
Task Calculator::calculate(const CalculationRequest& request)
{
    Logger::getInstance().info("Starting calculation");
    lit::MathRes calculationResult{};
    Task task{request};
    switch (request.operation)
    {
        case Operation::Add:
            calculationResult =
                lit::add(request.firstValue, request.secondValue);
            break;
        case Operation::Sub:
            calculationResult =
                lit::subtract(request.firstValue, request.secondValue);
            break;
        case Operation::Mul:
            calculationResult =
                lit::multiply(request.firstValue, request.secondValue);
            break;
        case Operation::Div:
            calculationResult =
                lit::divide(request.firstValue, request.secondValue);
            break;
        case Operation::Pow:
            calculationResult =
                lit::pow(request.firstValue, request.secondValue);
            break;
        case Operation::Fact:
            calculationResult = lit::factorial(request.firstValue);
            break;
        default:
            static_assert(true, "Unreachable state");
    }

    switch (calculationResult.error_)
    {
        case lit::ErrorStatus::kOverflow:
            task.status = Status::Overflow;
            break;
        case lit::ErrorStatus::kDivisionByZero:
            task.status = Status::DivideByZero;
            break;
        case lit::ErrorStatus::kInvalidNumber:
            task.status = Status::InvalidNumber;
            break;
        case lit::ErrorStatus::kOk:
            task.result = calculationResult.value_;
            task.status = Status::Success;
            break;
        default:
            task.status = Status::Error;
    }
    Logger::getInstance().info("Calculation finished");

    return task;
}
} // namespace calculator
