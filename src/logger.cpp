#include "logger.hpp"

#include "spdlog/spdlog.h"

namespace calculator
{
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
