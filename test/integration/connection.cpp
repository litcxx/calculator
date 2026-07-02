#include "config.hpp"
#include "database/connection.hpp"

#include <gtest/gtest.h>

using namespace calculator; // NOLINT

TEST(ConnectionTest, SuccessConnect)
{
    // Arrange & Act & Assert
    EXPECT_NO_THROW((Connection{test::kDBValidConfig}));
}

TEST(ConnectionTest, FailedConnect)
{
    // Arrange & Act & Assert
    EXPECT_THROW((Connection{test::kDBInvalidValidConfig}), std::runtime_error);
}

TEST(ConnectionTest, GetConnection)
{
    // Arrange
    Connection sut{test::kDBValidConfig};

    // Act
    auto* conn = sut.get();

    // Assert
    EXPECT_NE(conn, nullptr);
}

TEST(ConnectionTest, MoveConnection)
{
    // Arrange
    Connection sut{test::kDBValidConfig};

    // Act & assert
    Connection sut2(std::move(sut));

    void* oldConn = sut.get();
    void* newConn = sut2.get();
    EXPECT_EQ(oldConn, nullptr);
    EXPECT_NE(newConn, nullptr);

    sut = std::move(sut2);
    oldConn = sut.get();
    newConn = sut2.get();
    EXPECT_NE(oldConn, nullptr);
    EXPECT_EQ(newConn, nullptr);
}
