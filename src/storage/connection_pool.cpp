#include "storage/connection_pool.hpp"

#include "database/connection_guard.hpp"

#include <stdexcept>

namespace calculator
{
ConnectionPool::ConnectionPool(std::size_t count, const Config& config)
{
    for (std::size_t i = 0; i < count; ++i)
    {
        pool_.emplace_back(config);
    }
}

ConnectionGuard ConnectionPool::acquire()
{
    if (empty())
    {
        throw std::runtime_error("Pool has no connections");
    }

    ConnectionGuard connGuard(std::move(pool_.back()), *this);
    pool_.pop_back();
    return connGuard;
}

void ConnectionPool::release(Connection conn)
{
    pool_.push_back(std::move(conn));
}

bool ConnectionPool::empty()
{
    return pool_.empty();
}

} // namespace calculator
