#ifndef SIGNAL_HANDLER_HPP_
#define SIGNAL_HANDLER_HPP_

#include <atomic>
#include <functional>
#include <thread>

namespace calculator
{
// Blocks termination signals in the calling (main) thread and waits for them in
// a dedicated thread via sigwait. This keeps signal handling out of async-signal
// context: on delivery we may log, take locks and stop the io_context safely.
//
// IMPORTANT: construct FIRST, before starting any other threads, so the signal
// mask is inherited by every thread.
class SignalHandler
{
  public:
    SignalHandler();
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&) = delete;
    SignalHandler& operator=(SignalHandler&&) = delete;
    ~SignalHandler();

    // Launches the signal thread. onShutdown is invoked once a termination
    // signal arrives (e.g. io_context.stop()); it is passed here rather than to
    // the constructor because the io_context is created after signals are blocked.
    void start(std::function<void()> onShutdown = {});

    // Blocks the caller until a termination signal has been handled.
    void waitForShutdown();

    bool stopRequested() const noexcept;

  private:
    std::function<void()> onShutdown_;
    std::thread thread_;
    std::atomic<bool> stopRequested_{false};
};
} // namespace calculator

#endif
