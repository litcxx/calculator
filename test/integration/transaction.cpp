#include "database/transaction.hpp"

#include "config.hpp"
#include "database/connection_guard.hpp"
#include "storage/connection_pool.hpp"

#include <vector>

#include <gtest/gtest.h>

namespace calculator::test
{
TEST(TransactionTest, BeginSuccessTransaction)
{
    // Arrange
    ConnectionPool pool(1, TestConfig::config());

    // Act & Assert
    EXPECT_NO_THROW((Transaction(pool.acquire())));
}

TEST(TransactionTest, RecordAndFetchTask)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    ConnectionPool pool(1, TestConfig::config());
    Transaction sut(pool.acquire());

    // Act
    sut.record(task);
    const auto result = sut.fetch(request);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}

TEST(TransactionTest, FetchAllData)
{
    // Arrange
    const std::size_t count = 5;
    ConnectionPool pool(1, TestConfig::config());
    Transaction sut(pool.acquire());
    std::vector<Task> buf;

    // Act
    for (std::size_t i = 0; i < count; ++i)
    {
        const CalculationRequest request{static_cast<int>(i),
                                         static_cast<int>(i), Operation::Add};
        const Task task{request, static_cast<int>(i + i), Status::Success};
        sut.record(task);
    }
    sut.fetchAll([&buf](const Task& task) { buf.push_back(task); });

    // Assert
    ASSERT_EQ(buf.size(), count);
    for (std::size_t i = 0; i < count; ++i)
    {
        const CalculationRequest request{static_cast<int>(i),
                                         static_cast<int>(i), Operation::Add};
        const Task task{request, static_cast<int>(i + i), Status::Success};
        EXPECT_EQ(buf[i], task);
    }
}

TEST(TransactionTest, MoveTransaction)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    ConnectionPool pool(1, TestConfig::config());
    Transaction sut(pool.acquire());

    // Act
    sut.record(task);

    Transaction sut2(std::move(sut));
    const auto result2 = sut2.fetch(request);

    sut = std::move(sut2);
    const auto result = sut.fetch(request);

    // Assert
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), task);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}

TEST(TransactionTest, FetchReversedAdd)
{
    // Arrange
    const CalculationRequest request{1, 2, Operation::Add};
    const CalculationRequest reversedRequest{2, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    const Task reversedTask{reversedRequest, 2, Status::Success};
    ConnectionPool pool(1, TestConfig::config());
    Transaction sut(pool.acquire());

    // Act
    sut.record(task);
    const auto result = sut.fetch(reversedRequest);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), reversedTask);
}

TEST(TransactionTest, FetchReversedMul)
{
    // Arrange
    const CalculationRequest request{1, 2, Operation::Mul};
    const CalculationRequest reversedRequest{2, 1, Operation::Mul};
    const Task task{request, 2, Status::Success};
    const Task reversedTask{reversedRequest, 2, Status::Success};
    ConnectionPool pool(1, TestConfig::config());
    Transaction sut(pool.acquire());

    // Act
    sut.record(task);
    const auto result = sut.fetch(reversedRequest);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), reversedTask);
}
} // namespace calculator::test
