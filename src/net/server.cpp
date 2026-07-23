#include "net/server.hpp"

#include <istream>
#include <memory>
#include <string>
#include <utility>

namespace calculator
{
namespace
{
using boost::asio::ip::tcp;

// One client connection: read a request line, answer with the result, repeat
// until the client closes. Kept alive by shared_from_this during async ops.
class Session : public std::enable_shared_from_this<Session>
{
  public:
    Session(tcp::socket socket, RequestHandler& handler) :
        socket_{std::move(socket)}, handler_{handler}
    {}

    void start()
    {
        readLine();
    }

  private:
    void readLine()
    {
        auto self = shared_from_this();
        boost::asio::async_read_until(
            socket_, buffer_, '\n',
            [this, self](const boost::system::error_code& ec, std::size_t)
            {
                if (ec)
                {
                    return; // client closed or error: drop the session
                }
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);
                writeResponse(handler_.handle(line));
            });
    }

    void writeResponse(std::string response)
    {
        response_ = std::move(response);
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_, boost::asio::buffer(response_),
            [this, self](const boost::system::error_code& ec, std::size_t)
            {
                if (ec)
                {
                    return;
                }
                readLine(); // serve the next request on this connection
            });
    }

    tcp::socket socket_;
    RequestHandler& handler_;
    boost::asio::streambuf buffer_;
    std::string response_;
};
} // namespace

Server::Server(boost::asio::io_context& io, std::uint16_t port,
               RequestHandler& handler) :
    acceptor_{io, tcp::endpoint(tcp::v4(), port)}, handler_{handler}
{
    doAccept();
}

std::uint16_t Server::port() const
{
    return acceptor_.local_endpoint().port();
}

void Server::doAccept()
{
    acceptor_.async_accept(
        [this](const boost::system::error_code& ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<Session>(std::move(socket), handler_)->start();
            }
            doAccept(); // accept the next client
        });
}
} // namespace calculator
