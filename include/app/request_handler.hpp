#ifndef REQUEST_HANDLER_HPP_
#define REQUEST_HANDLER_HPP_

#include "core/i_calculator.hpp"
#include "core/i_parser.hpp"
#include "core/i_repository.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace calculator
{
// Turns a single JSON request line into a response line. Reuses the domain
// layer (parser/repository/calculator) and formats the result via toResponse.
// Parsing/computation errors are turned into an error response, never thrown,
// so a malformed client request cannot tear down the connection.
class RequestHandler
{
  public:
    RequestHandler(std::unique_ptr<IRepository> repository,
                   std::unique_ptr<IParser> parser,
                   std::unique_ptr<ICalculator> calculator);

    std::string handle(std::string_view requestJson);

  private:
    std::unique_ptr<IRepository> repository_;
    std::unique_ptr<IParser> parser_;
    std::unique_ptr<ICalculator> calculator_;
};
} // namespace calculator

#endif
