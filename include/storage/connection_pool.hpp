#ifndef CONNECTION_POOL_HPP_
#define CONNECTION_POOL_HPP_

#include "database/connection.hpp"
#include "database/connection_guard.hpp"

#include <deque>

namespace calculator
{
class ConnectionPool
{
  public:
    ConnectionPool() = default;
    explicit ConnectionPool(std::size_t count, const Config& config);
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ConnectionPool(ConnectionPool&&) noexcept = default;
    ConnectionPool& operator=(ConnectionPool&&) noexcept = default;
    ~ConnectionPool() = default;

    ConnectionGuard acquire();
    void release(Connection conn);
    bool empty();

  private:
    std::deque<Connection> pool_;
};
} // namespace calculator

#endif
