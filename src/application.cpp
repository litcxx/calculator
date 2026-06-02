#include "application.hpp"

#include "litmath/math.hpp"
#include "parser.hpp"

#include <print>
#include <stdexcept>

namespace
{
std::string toErrorMsg(lit::ErrorStatus ec)
{
    switch (ec)
    {
        case lit::ErrorStatus::kOverflow:
            return "Overflow";
        case lit::ErrorStatus::kDivisionByZero:
            return "Division by zero";
        case lit::ErrorStatus::kInvalidNumber:
            return "invalid number";
        default:
            return "Calculation error";
    }
}
} // namespace

namespace calculator
{
void Application::run(int argc, char** argv)
{
    getTask(argc, argv);
    makeCalculate();
    printResult();
}

void Application::getTask(int argc, char** argv)
{
    if (argc != 2)
        throw std::logic_error("calc: Use --help for more information.");
    task_.request = Parser::Parse(argv[1]);
}

void Application::makeCalculate()
{
    lit::MathRes res{};
    auto& request = task_.request;
    switch (task_.request.operation)
    {
        case Operation::Add:
            res = lit::add(request.firstValue, request.secondValue);
            break;
        case Operation::Sub:
            res = lit::subtract(request.firstValue, request.secondValue);
            break;
        case Operation::Mul:
            res = lit::multiply(request.firstValue, request.secondValue);
            break;
        case Operation::Div:
            res = lit::divide(request.firstValue, request.secondValue);
            break;
        case Operation::Pow:
            res = lit::pow(request.firstValue, request.secondValue);
            break;
        case Operation::Fact:
            res = lit::factorial(request.firstValue);
            break;
        default:
            throw std::logic_error(
                "Unsupported operation. Use --help for more information.");
    }

    if (res.error_ != lit::ErrorStatus::kOk)
    {
        throw std::logic_error(toErrorMsg(res.error_));
    }
    task_.result = res.value_;
    task_.status = Status::Success;
}

void Application::printResult() const
{
    std::println("{}", task_.result);
}

} // namespace calculator
