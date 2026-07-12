#include "database/connection_guard.hpp"

#include "storage/connection_pool.hpp"

#include <utility>

namespace calculator
{

ConnectionGuard::ConnectionGuard(Connection conn, ConnectionPool& pool) noexcept
    : conn_{std::move(conn)}, pool_{pool}
{}

ConnectionGuard::ConnectionGuard(ConnectionGuard&& rhs) noexcept :
    conn_{std::exchange(rhs.conn_, {})}, pool_{rhs.pool_}
{}

ConnectionGuard& ConnectionGuard::operator=(ConnectionGuard&& rhs) noexcept
{
    if (this != &rhs)
    {
        release();
        conn_ = std::exchange(rhs.conn_, {});
    }
    return *this;
}

ConnectionGuard::~ConnectionGuard()
{
    release();
}

void* ConnectionGuard::get()
{
    return conn_.get();
}

void ConnectionGuard::release()
{
    if (conn_)
    {
        pool_.release(std::move(conn_));
    }
}

} // namespace calculator
