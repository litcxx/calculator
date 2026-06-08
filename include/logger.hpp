#ifndef LOGGER_H_
#define LOGGER_H_

#include <string_view>

namespace calculator
{
class Logger
{
  public:
    static Logger& getInstance();

    void info(std::string_view msg);
    void warn(std::string_view msg);
    void error(std::string_view msg);

  private:
    Logger();
};
} // namespace calculator

#endif
