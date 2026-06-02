#pragma once

#include <format>
#include <string_view>

namespace calculator
{
class Logger
{
  public:
    Logger();

    static Logger& getInstance();

    template <typename... Types>
    void info(std::format_string<Types...> fmt, Types&&... args)
    {
        return log_info(std::format(fmt, std::forward<Types>(args)...));
    }

    template <typename... Types>
    void warn(std::format_string<Types...> fmt, Types&&... args)
    {
        return log_warn(std::format(fmt, std::forward<Types>(args)...));
    }

    template <typename... Types>
    void error(std::format_string<Types...> fmt, Types&&... args)
    {
        return log_error(std::format(fmt, std::forward<Types>(args)...));
    }

  private:
    void log_info(std::string_view msg);
    void log_warn(std::string_view msg);
    void log_error(std::string_view msg);
};
} // namespace calculator
