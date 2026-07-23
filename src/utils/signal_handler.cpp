#define _POSIX_C_SOURCE 200809L // must precede includes: exposes POSIX sigwait/pthread_sigmask

#include "utils/signal_handler.hpp"

#include <csignal>
#include <pthread.h>
#include <unistd.h>

namespace calculator
{
namespace
{
sigset_t terminationSet()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    return set;
}
} // namespace

SignalHandler::SignalHandler()
{
    sigset_t set = terminationSet();
    pthread_sigmask(SIG_BLOCK, &set, nullptr); // inherited by every future thread
}

void SignalHandler::start(std::function<void()> onShutdown)
{
    onShutdown_ = std::move(onShutdown); // set before the thread starts: no data race
    thread_ = std::thread(
        [this]
        {
            sigset_t set = terminationSet();
            int sig = 0;
            sigwait(&set, &sig); // wait for SIGTERM/SIGINT
            stopRequested_.store(true, std::memory_order_relaxed);
            if (onShutdown_)
            {
                onShutdown_();
            }
        });
}

void SignalHandler::waitForShutdown()
{
    if (thread_.joinable())
    {
        thread_.join(); // blocks until the signal thread handled a signal, no spin
    }
}

bool SignalHandler::stopRequested() const noexcept
{
    return stopRequested_.load(std::memory_order_relaxed);
}

SignalHandler::~SignalHandler()
{
    if (thread_.joinable())
    {
        // No signal arrived (e.g. startup threw): wake sigwait so we can join.
        if (!stopRequested_.load(std::memory_order_relaxed))
        {
            ::kill(::getpid(), SIGTERM);
        }
        thread_.join();
    }
}
} // namespace calculator
