#include "application.hpp"

#include "litmath/math.hpp"
#include "logger.hpp"
#include "parser.hpp"

#include <cstring>
#include <format>
#include <print>
#include <stdexcept>

namespace
{
struct ErrorTranslator
{
    static std::string toErrorMsg(lit::ErrorStatus ec)
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
};
} // namespace

namespace calculator
{
void Application::run(int argc, char** argv)
{
    Logger::getInstance().info("Application started");
    getTask(argc, argv);
    makeCalculate();
    printResult();
    Logger::getInstance().info("Application finished successfully");
}

void Application::getTask(int argc, char** argv)
{
    if (argc != 2)
    {
        task_.status = Status::Error;
        Logger::getInstance().error("Invalid number of arguments: {}", argc);
        throw std::logic_error("calc: Use --help for more information.");
    }

    if (std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0)
    {
        throw std::format(
            "Usage: {} \'{{\"first\": [value], \"second\": [value], "
            "\"operation\": [op]}}\'\n"
            "Operation supports: add, sub, div, mul, pow, fact(requires only "
            "\"first\" and \"op\")",
            argv[0]);
    }

    try
    {
        Logger::getInstance().info("Parsing task from: {}", argv[1]);
        task_.request = Parser::Parse(argv[1]);
    }
    catch (const std::exception& e)
    {
        task_.status = Status::Error;
        Logger::getInstance().error("Parsing error: {}", e.what());
        throw;
    }
}

void Application::makeCalculate()
{
    Logger::getInstance().info("Starting calculation");
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
            task_.status = Status::Error;
            Logger::getInstance().error("Unsupported operation encountered");
            throw std::logic_error(
                "Unsupported operation. Use --help for more information.");
    }

    if (res.error_ != lit::ErrorStatus::kOk)
    {
        task_.status = Status::Error;
        std::string msg = ErrorTranslator::toErrorMsg(res.error_);
        Logger::getInstance().error("Calculation error: {}", msg);
        throw std::logic_error(msg);
    }
    task_.result = res.value_;
    task_.status = Status::Success;
    Logger::getInstance().info("Calculation successful: result = {}",
                               task_.result);
}

void Application::printResult() const
{
    std::println("{}", task_.result);
}

} // namespace calculator
