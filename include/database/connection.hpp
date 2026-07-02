#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <memory>
#include <string>

namespace calculator
{
struct Config
{
    std::string host;
    std::string port;
    std::string dbname;
    std::string username;
    std::string password;
};

class Connection
{
  public:
    Connection();
    explicit Connection(const Config& config);
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept;
    Connection& operator=(Connection&&) noexcept;
    ~Connection();
    void* get();
    explicit operator bool() const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace calculator
#endif
