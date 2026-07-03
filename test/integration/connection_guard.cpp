#include "database/connection_guard.hpp"

#include "config.hpp"
#include "storage/connection_pool.hpp"

#include <gtest/gtest.h>

namespace calculator::test
{
TEST(ConnectionGuardTest, Dtor)
{
    // Arrange
    ConnectionPool pool(1, TestConfig::config());

    // Act & Assert
    ASSERT_FALSE(pool.empty());
    {
        auto connGuard(pool.acquire());
        ASSERT_TRUE(pool.empty());
    }
    ASSERT_FALSE(pool.empty());
}
} // namespace calculator::test
