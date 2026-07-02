#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include "core/i_calculator.hpp"
#include "core/i_parser.hpp"
#include "core/i_printer.hpp"
#include "core/i_repository.hpp"

#include <memory>

class Logger;

namespace calculator
{
class Application
{
  public:
    explicit Application(std::unique_ptr<IRepository> repository,
                         std::unique_ptr<IParser> parser,
                         std::unique_ptr<ICalculator> calculator,
                         std::unique_ptr<IPrinter> printer);
    void run(int argc, char** argv);

  private:
    std::unique_ptr<IRepository> repository_;
    std::unique_ptr<IParser> parser_;
    std::unique_ptr<ICalculator> calculator_;
    std::unique_ptr<IPrinter> printer_;
};
} // namespace calculator

#endif
