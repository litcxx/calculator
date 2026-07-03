#include "app/calculator.hpp"

#include "utils/config.hpp"

#include <climits>

#include <gtest/gtest.h>

namespace calculator::test
{
TEST(CalculatorTest, AddSuccess)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result, task);
}

TEST(CalculatorTest, SubtractionSuccess)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Sub};
    const Task task{request, 0, Status::Success};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result, task);
}

TEST(CalculatorTest, MultiplicationSuccess)
{
    // Arrange
    const CalculationRequest request{2, 2, Operation::Mul};
    const Task task{request, 4, Status::Success};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result, task);
}

TEST(CalculatorTest, DivisionSuccess)
{
    // Arrange
    const CalculationRequest request{2, 2, Operation::Div};
    const Task task{request, 1, Status::Success};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result, task);
}

TEST(CalculatorTest, PowerSuccess)
{
    // Arrange
    const CalculationRequest request{2, 2, Operation::Pow};
    const Task task{request, 4, Status::Success};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result, task);
}

TEST(CalculatorTest, FactorialSuccess)
{
    // Arrange
    const CalculationRequest request{2, 0, Operation::Fact};
    const Task task{request, 2, Status::Success};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result, task);
}

TEST(CalculatorTest, Overlofw)
{
    // Arrange
    const CalculationRequest request{INT_MAX, INT_MAX, Operation::Add};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result.status, Status::Overflow);
}

TEST(CalculatorTest, DivideByZero)
{
    // Arrange
    const CalculationRequest request{1, 0, Operation::Div};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result.status, Status::DivideByZero);
}

TEST(CalculatorTest, InvalidNumber)
{
    // Arrange
    const CalculationRequest request{-1, 0, Operation::Fact};
    Calculator sut;

    // Act
    const Task result = sut.calculate(request);

    // Assert
    EXPECT_EQ(result.status, Status::InvalidNumber);
}
} // namespace calculator::test
