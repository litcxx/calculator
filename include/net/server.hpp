#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "app/request_handler.hpp"

#include <boost/asio.hpp>

#include <cstdint>

namespace calculator
{
// TCP server: accepts connections and answers each request line with the
// calculation result via RequestHandler. Drives an externally owned io_context;
// the owner controls the lifecycle through io.run() / io.stop().
class Server
{
  public:
    Server(boost::asio::io_context& io, std::uint16_t port,
           RequestHandler& handler);

    std::uint16_t port() const;

  private:
    void doAccept();

    boost::asio::ip::tcp::acceptor acceptor_;
    RequestHandler& handler_;
};
} // namespace calculator

#endif
