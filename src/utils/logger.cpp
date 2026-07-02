#include "utils/logger.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <memory>

namespace calculator
{
Logger::Logger()
{
    auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger =
        std::make_shared<spdlog::logger>("stderr_logger", consoleSink);
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::info(std::string_view msg)
{
    spdlog::info(msg);
}

void Logger::warn(std::string_view msg)
{
    spdlog::warn(msg);
}

void Logger::error(std::string_view msg)
{
    spdlog::error(msg);
}
} // namespace calculator
