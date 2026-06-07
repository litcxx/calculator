#pragma once

#include <format>
#include <string_view>

namespace calculator
{
class Logger
{
  public:
    static Logger& getInstance();

    template <typename... Types>
    void info(std::format_string<Types...> fmt, Types&&... args)
    {
        return logInfo(std::format(fmt, std::forward<Types>(args)...));
    }

    template <typename... Types>
    void warn(std::format_string<Types...> fmt, Types&&... args)
    {
        return logWarn(std::format(fmt, std::forward<Types>(args)...));
    }

    template <typename... Types>
    void error(std::format_string<Types...> fmt, Types&&... args)
    {
        return logError(std::format(fmt, std::forward<Types>(args)...));
    }

  private:
    Logger();

    static void logInfo(std::string_view msg);
    static void logWarn(std::string_view msg);
    static void logError(std::string_view msg);
};
} // namespace calculator
