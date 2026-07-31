#include "net/server.hpp"

#include "app/calculator.hpp"
#include "app/request_handler.hpp"
#include "core/i_repository.hpp"
#include "io/parser.hpp"
#include "utils/config.hpp"

#include <boost/asio.hpp>

#include <cstdint>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace calculator::test
{
namespace
{
// In-memory IRepository so the network path needs no database.
class FakeRepository : public IRepository
{
  public:
    std::optional<Task> get(const CalculationRequest& request) override
    {
        for (const auto& task : tasks_)
        {
            if (task.request == request)
            {
                return task;
            }
        }
        return std::nullopt;
    }
    void save(const Task& task) override
    {
        tasks_.push_back(task);
    }

  private:
    std::vector<Task> tasks_;
};

class NetworkTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto repository = std::make_unique<FakeRepository>();
        handler_ = std::make_unique<RequestHandler>(
            std::move(repository), std::make_unique<Parser>(),
            std::make_unique<Calculator>());
        server_ = std::make_unique<Server>(io_, 0, *handler_);
        port_ = server_->port();
        ioThread_ = std::thread([this] { io_.run(); });
    }

    void TearDown() override
    {
        io_.stop();
        if (ioThread_.joinable())
        {
            ioThread_.join();
        }
    }

    // Test client: sends one request line, returns the response line (no '\n').
    std::string request(std::string_view json)
    {
        namespace asio = boost::asio;
        using asio::ip::tcp;

        asio::io_context clientIo;
        tcp::socket socket(clientIo);
        socket.connect(
            tcp::endpoint(asio::ip::make_address("127.0.0.1"), port_));

        const std::string out = std::string(json) + "\n";
        asio::write(socket, asio::buffer(out));

        asio::streambuf buffer;
        asio::read_until(socket, buffer, '\n');
        std::istream is(&buffer);
        std::string line;
        std::getline(is, line);
        return line;
    }

    boost::asio::io_context io_;
    std::unique_ptr<RequestHandler> handler_;
    std::unique_ptr<Server> server_;
    std::uint16_t port_{0};
    std::thread ioThread_;
};
} // namespace

TEST_F(NetworkTest, ReturnsCalculationResult)
{
    EXPECT_EQ(request(R"({"first":2,"second":3,"operation":"add"})"),
              "Success : 5");
}

TEST_F(NetworkTest, ReturnsErrorForDivisionByZero)
{
    EXPECT_EQ(request(R"({"first":10,"second":0,"operation":"div"})"),
              "Error: Divide by zero");
}

TEST_F(NetworkTest, RepeatedRequestReturnsSameResult)
{
    EXPECT_EQ(request(R"({"first":7,"second":8,"operation":"mul"})"),
              "Success : 56");
    EXPECT_EQ(request(R"({"first":7,"second":8,"operation":"mul"})"),
              "Success : 56");
}
} // namespace calculator::test
