#include "app/request_handler.hpp"

#include "io/response.hpp"
#include "utils/config.hpp"

#include <exception>

namespace calculator
{
RequestHandler::RequestHandler(std::unique_ptr<IRepository> repository,
                               std::unique_ptr<IParser> parser,
                               std::unique_ptr<ICalculator> calculator) :
    repository_{std::move(repository)}, parser_{std::move(parser)},
    calculator_{std::move(calculator)}
{}

std::string RequestHandler::handle(std::string_view requestJson)
{
    try
    {
        const CalculationRequest request = parser_->parse(requestJson);

        if (const auto task = repository_->get(request); task.has_value())
        {
            return toResponse(task.value());
        }

        const Task task = calculator_->calculate(request);
        repository_->save(task);
        return toResponse(task);
    }
    catch (const std::exception& e)
    {
        return std::string("Error: ") + e.what() + "\n";
    }
}
} // namespace calculator
