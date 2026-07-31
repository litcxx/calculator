// _POSIX_C_SOURCE must precede every include: it exposes POSIX sigwait and
// pthread_sigmask under strict -std=c++17.

// NOLINTNEXTLINE(bugprone-reserved-identifier, cppcoreguidelines-macro-usage)
#define _POSIX_C_SOURCE 200809L

#include "utils/signal_handler.hpp"

#include <pthread.h>
#include <unistd.h>

#include <csignal>

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
    const sigset_t set = terminationSet();
    // inherited by every future thread
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
}

void SignalHandler::start(std::function<void()> onShutdown)
{
    // set before the thread starts: no data race
    onShutdown_ = std::move(onShutdown);
    thread_ = std::thread([this] {
        const sigset_t set = terminationSet();
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
        // blocks until the signal thread handled a signal, no spin
        thread_.join();
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
