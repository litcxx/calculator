#include "utils/logger.hpp"
#include "utils/signal_handler.hpp"

#include <cstdlib>
#include <exception>

using namespace calculator; // NOLINT

// The service runs until it receives SIGTERM (or SIGINT), then shuts down
// gracefully. Request processing over the network is added in a later step;
// here the worker (main) thread simply waits for the termination signal.
int main()
{
    try
    {
        SignalHandler signals; // (1) block signals in the main thread FIRST
        signals.start();       // (2) dedicated signal thread (thread #1)

        Logger::getInstance().info("Service started (waiting for SIGTERM)");

        signals.waitForShutdown(); // worker thread (#2) blocks until signalled

        Logger::getInstance().info("SIGTERM received, shutting down gracefully");
    }
    catch (const std::exception& ec)
    {
        Logger::getInstance().error(ec.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
