#include "app/application.hpp"
#include "app/calculator.hpp"
#include "io/parser.hpp"
#include "io/stdout_printer.hpp"
#include "storage/cache.hpp"
#include "storage/connection_pool.hpp"
#include "storage/repository.hpp"
#include "utils/logger.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

using namespace calculator; // NOLINT

int main(int argc, char** argv)
{
    try
    {
        Cache cache;
        ConnectionPool pool(1, Config{"localhost", "5433", "calculatordb",
                                      "user123", "user123"});

        auto repository =
            std::make_unique<Repository>(std::move(pool), std::move(cache));

        Application application(
            std::move(repository), std::make_unique<Parser>(),
            std::make_unique<Calculator>(), std::make_unique<StdoutPrinter>());
        application.run(argc, argv);
    }
    catch (const std::exception& ec)
    {
        calculator::Logger::getInstance().error(ec.what());
        return EXIT_FAILURE;
    }
    catch (const std::string& str)
    {
        std::cout << str << '\n';
    }
    catch (...)
    {
        calculator::Logger::getInstance().error("Unknown error\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
