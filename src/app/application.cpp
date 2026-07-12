#include "app/application.hpp"

#include "core/i_calculator.hpp"
#include "core/i_parser.hpp"
#include "core/i_printer.hpp"
#include "core/i_repository.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"

namespace calculator
{
Application::Application(std::unique_ptr<IRepository> repository,
                         std::unique_ptr<IParser> parser,
                         std::unique_ptr<ICalculator> calculator,
                         std::unique_ptr<IPrinter> printer) :
    repository_{std::move(repository)}, parser_{std::move(parser)},
    calculator_{std::move(calculator)}, printer_{std::move(printer)}
{}

void Application::run(int argc, char** argv)
{
    Logger::getInstance().info("Application started");

    // INPUT
    parser_->validateInput(argc, argv);
    const CalculationRequest request = parser_->parse(argv[1]); // NOLINT

    // CACHE HIT
    if (const auto task = repository_->get(request); task.has_value())
    {
        Logger::getInstance().info("Find result in cache");
        printer_->print(task.value());
        return;
    }

    // CACHE MISS
    const Task task = calculator_->calculate(request);
    repository_->save(task);

    // OUTPUT
    printer_->print(task);
    Logger::getInstance().info("Application finished successfully");
}
} // namespace calculator
