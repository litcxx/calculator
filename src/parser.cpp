#include "parser.hpp"

#include "config.hpp"
#include "nlohmann/json.hpp"

#include <string_view>

using json = nlohmann::json;

namespace calculator
{
CalculationRequest Parser::Parse(std::string_view line)
{
    CalculationRequest res;
    try
    {
        const auto parsed_data = json::parse(line);
        std::string_view operation =
            parsed_data.at("operation").get_ref<const std::string&>();

        res.operation = toOperation(operation);
        switch (res.operation)
        {
            case Operation::Fact:
                res.firstValue = parsed_data.at("first");
                if (parsed_data.count("second") != 0)
                    throw std::logic_error("calc: Invalid json format. Use "
                                           "--help for more information.");
                break;
            default:
                res.firstValue = parsed_data.at("first");
                res.secondValue = parsed_data.at("second");
        }
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
    return res;
}

Operation Parser::toOperation(std::string_view operation)
{
    if (operation == "add")
        return Operation::Add;
    if (operation == "sub")
        return Operation::Sub;
    if (operation == "mul")
        return Operation::Mul;
    if (operation == "div")
        return Operation::Div;
    if (operation == "pow")
        return Operation::Pow;
    if (operation == "fact")
        return Operation::Fact;
    throw std::logic_error(
        "Unsupported operation. Use --help for more information.");
}
} // namespace calculator
