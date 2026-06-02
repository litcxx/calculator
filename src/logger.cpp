#include "logger.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <memory>

namespace calculator
{
Logger::Logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger =
        std::make_shared<spdlog::logger>("stderr_logger", console_sink);
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::log_info(std::string_view msg)
{
    spdlog::info(msg);
}

void Logger::log_warn(std::string_view msg)
{
    spdlog::warn(msg);
}

void Logger::log_error(std::string_view msg)
{
    spdlog::error(msg);
}
} // namespace calculator
