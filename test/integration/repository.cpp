#include "storage/repository.hpp"

#include "config.hpp"
#include "database/connection.hpp"
#include "database/connection_guard.hpp"
#include "database/transaction.hpp"

#include <fmt/format.h>
#include <postgresql/libpq-fe.h>

#include <cstdint>
#include <memory>
#include <random>

#include <gtest/gtest.h>

using namespace calculator; // NOLINT

namespace
{
void clearValue(int firstValue, int secondValue, int sum)
{
    Connection conn(test::kDBValidConfig);
    auto* connPtr = static_cast<PGconn*>(conn.get());
    PGresult* res = PQexec(
        connPtr, fmt::format("DELETE FROM tasks WHERE first_value={} AND "
                             "second_value={} AND result={};",
                             firstValue, secondValue, sum)
                     .c_str());
    PQclear(res);
}
} // namespace

TEST(RepositoryTest, GetTaskFromWarmupCache)
{
    // Arrange
    std::random_device rand;
    std::uniform_int_distribution<int> dist(INT16_MIN, INT16_MAX);
    const int firstValue = dist(rand);
    const int secondValue = dist(rand);
    const int sum = firstValue + secondValue;
    const CalculationRequest request{firstValue, secondValue, Operation::Add};
    const Task task{request, sum, Status::Success};
    auto clearGuard =
        std::shared_ptr<void>(nullptr, [firstValue, secondValue, sum](void*) {
            clearValue(firstValue, secondValue, sum);
        });

    ConnectionPool pool;
    {
        Transaction transction(
            ConnectionGuard(Connection(test::kDBValidConfig), pool));
        transction.record(task);
        transction.commit();
    }

    // Act
    Repository sut(ConnectionPool(1, test::kDBValidConfig), Cache());
    auto result = sut.get(request);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}

TEST(RepositoryTest, SaveTaskAndGetFromCache)
{
    // Arrange
    std::random_device rand;
    std::uniform_int_distribution<int> dist(INT16_MIN, INT16_MAX);
    const int firstValue = dist(rand);
    const int secondValue = dist(rand);
    const int sum = firstValue + secondValue;
    const CalculationRequest request{firstValue, secondValue, Operation::Add};
    const Task task{request, sum, Status::Success};
    auto clearGuard =
        std::shared_ptr<void>(nullptr, [firstValue, secondValue, sum](void*) {
            clearValue(firstValue, secondValue, sum);
        });

    // Act
    Repository sut(ConnectionPool(1, test::kDBValidConfig), Cache());
    sut.save(task);
    auto result = sut.get(request);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}
