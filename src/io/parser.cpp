#include "io/parser.hpp"

#include "nlohmann/json.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"

#include <fmt/format.h>

#include <cstring>
#include <stdexcept>
#include <string_view>

using json = nlohmann::json;
namespace
{
calculator::Operation toOperation(std::string_view operation)
{

    if (operation == "add")
    {
        return calculator::Operation::Add;
    }
    if (operation == "sub")
    {
        return calculator::Operation::Sub;
    }
    if (operation == "mul")
    {
        return calculator::Operation::Mul;
    }
    if (operation == "div")
    {
        return calculator::Operation::Div;
    }
    if (operation == "pow")
    {
        return calculator::Operation::Pow;
    }
    if (operation == "fact")
    {
        return calculator::Operation::Fact;
    }
    throw std::logic_error(
        "Unsupported operation. Use --help for more information.");
}
} // namespace

namespace calculator
{
void Parser::validateInput(int argc, char** argv)
{
    if (argc != 2 || argv == nullptr)
    {
        Logger::getInstance().error("Invalid number of arguments");
        throw std::logic_error("calc: Use --help for more information.");
    }

    if (std::strcmp(argv[1], "--help") == 0 || // NOLINT
        std::strcmp(argv[1], "-h") == 0)       // NOLINT
    {
        throw std::string(
            "Usage: calc \'{\"first\": [value], \"second\": [value], "
            "\"operation\": [op]}\'\n"
            "Operation supports: add, sub, div, mul, pow, fact(requires "
            "only "
            "\"first\" and \"op\")");
    }
}

CalculationRequest Parser::parse(std::string_view line)
{
    try
    {
        CalculationRequest request;

        const auto parsedData = json::parse(line);
        const std::string_view operation =
            parsedData.at("operation").get_ref<const std::string&>();

        request.operation = toOperation(operation);
        switch (request.operation)
        {
            case Operation::Fact:
                request.firstValue = parsedData.at("first");
                if (parsedData.count("second") != 0) // NOLINT
                {
                    throw std::logic_error("calc: Invalid json format. Use "
                                           "--help for more information.");
                }
                break;
            default:
                request.firstValue = parsedData.at("first");
                request.secondValue = parsedData.at("second");
        }

        return request;
    }
    catch (const json::exception& e)
    {
        throw std::logic_error(
            "calc: Invalid json format. Use --help for more information.");
    }
    catch (...)
    {
        throw;
    }
}
} // namespace calculator
