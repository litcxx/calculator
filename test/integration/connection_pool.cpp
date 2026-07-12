#include "storage/connection_pool.hpp"

#include "config.hpp"

#include <cstddef>

#include <gtest/gtest.h>

namespace calculator::test
{
TEST(ConnectionPoolTest, InitEmptyPool)
{
    // Arrage
    ConnectionPool sut;

    // Assert
    EXPECT_TRUE(sut.empty());
    EXPECT_THROW(sut.acquire(), std::runtime_error);
}

TEST(ConnectionPoolTest, InitMultipleConnection)
{
    // Arrage
    const std::size_t count = 10;
    ConnectionPool sut(count, TestConfig::config());

    // Act & Assert
    ASSERT_FALSE(sut.empty());

    for (std::size_t i = 0; i < count; ++i)
    {
        auto conn = sut.acquire();
        EXPECT_NE(conn.get(), nullptr);
    }
}

TEST(ConnectionPoolTest, ReleaseConnection)
{
    // Arrage
    Connection conn(TestConfig::config());
    ConnectionPool sut;

    // Act & Assert
    ASSERT_TRUE(sut.empty());
    sut.release(std::move(conn));
    EXPECT_FALSE(sut.empty());
}
} // namespace calculator::test
