#include "storage/cache.hpp"

#include "utils/config.hpp"

#include <gtest/gtest.h>

using namespace calculator; // NOLINT

TEST(CacheTest, InsertElement)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    Cache sut;

    // Act
    sut.insert(task);

    // Assert
    const auto result = sut.get(request);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}

TEST(CacheTest, GetNonExistentElement)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    Cache sut;

    // Act
    const auto result = sut.get(request);

    // Assert
    ASSERT_FALSE(result.has_value());
}

TEST(CacheTest, GetReverseRequestForAdd)
{
    // Arrange
    const CalculationRequest request{1, 0, Operation::Add};
    const CalculationRequest reversedRequest{0, 1, Operation::Add};
    const Task task{request, 1, Status::Success};
    Cache sut;

    // Act
    sut.insert(task);
    const auto result = sut.get(reversedRequest);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}

TEST(CacheTest, GetReverseRequestForMul)
{
    // Arrange
    const CalculationRequest request{1, 2, Operation::Mul};
    const CalculationRequest reversedRequest{2, 1, Operation::Mul};
    const Task task{request, 2, Status::Success};
    Cache sut;

    // Act
    sut.insert(task);
    const auto result = sut.get(reversedRequest);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), task);
}
