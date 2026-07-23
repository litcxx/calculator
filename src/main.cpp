#include "app/calculator.hpp"
#include "app/request_handler.hpp"
#include "database/db_config.hpp"
#include "io/parser.hpp"
#include "net/server.hpp"
#include "storage/cache.hpp"
#include "storage/connection_pool.hpp"
#include "storage/repository.hpp"
#include "utils/logger.hpp"
#include "utils/signal_handler.hpp"

#include <boost/asio.hpp>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>

using namespace calculator; // NOLINT

namespace
{
std::uint16_t serverPort()
{
    if (const char* env = std::getenv("CALC_PORT")) // NOLINT
    {
        return static_cast<std::uint16_t>(std::stoul(env));
    }
    return 5555;
}
} // namespace

// Long-running service: a dedicated thread waits for SIGTERM (thread #1) while
// the main thread runs the Asio event loop serving clients (thread #2). On
// signal the loop is stopped and the process shuts down gracefully.
int main()
{
    try
    {
        SignalHandler signals; // block signals in the main thread FIRST

        const Config config = getConfig();
        auto repository =
            std::make_unique<Repository>(ConnectionPool(1, config), Cache{});
        RequestHandler handler(std::move(repository),
                               std::make_unique<Parser>(),
                               std::make_unique<Calculator>());

        boost::asio::io_context io;
        Server server(io, serverPort(), handler);

        signals.start([&io] { io.stop(); }); // signal thread (#1) stops the loop
        Logger::getInstance().info("Listening on port " +
                                   std::to_string(server.port()));

        io.run(); // worker thread (#2): accept and process requests

        Logger::getInstance().info("SIGTERM received, shutting down gracefully");
    }
    catch (const std::exception& ec)
    {
        Logger::getInstance().error(ec.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
