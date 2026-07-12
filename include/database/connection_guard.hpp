#ifndef CONNECTION_LEASE_HPP_
#define CONNECTION_LEASE_HPP_

#include "database/connection.hpp"

namespace calculator
{
class ConnectionPool;
class ConnectionGuard
{
  public:
    explicit ConnectionGuard(Connection conn, ConnectionPool& pool) noexcept;
    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;
    ConnectionGuard(ConnectionGuard&& rhs) noexcept;
    ConnectionGuard& operator=(ConnectionGuard&& rhs) noexcept;
    ~ConnectionGuard();

    void* get();

  private:
    void release();

    Connection conn_;
    ConnectionPool& pool_; // NOLINT
};
} // namespace calculator

#endif
