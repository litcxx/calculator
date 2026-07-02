#include "database/connection_guard.hpp"

#include "config.hpp"
#include "storage/connection_pool.hpp"

#include <gtest/gtest.h>

using namespace calculator; // NOLINT

TEST(ConnectionLeaseTest, Dtor)
{
    // Arrange
    ConnectionPool pool(1, test::kDBValidConfig);

    // Act & Assert
    ASSERT_FALSE(pool.empty());
    {
        auto connGuard(pool.acquire());
        ASSERT_TRUE(pool.empty());
    }
    ASSERT_FALSE(pool.empty());
}
