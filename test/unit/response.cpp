#include "io/response.hpp"

#include "utils/config.hpp"

#include <gtest/gtest.h>

namespace calculator::test
{
TEST(ToResponseTest, SuccessIncludesResult)
{
    const Task task{{2, 3, Operation::Add}, 5, Status::Success};
    EXPECT_EQ(toResponse(task), "Success : 5\n");
}

TEST(ToResponseTest, DivideByZero)
{
    const Task task{{10, 0, Operation::Div}, 0, Status::DivideByZero};
    EXPECT_EQ(toResponse(task), "Error: Divide by zero\n");
}

TEST(ToResponseTest, Overflow)
{
    const Task task{{2, 100, Operation::Pow}, 0, Status::Overflow};
    EXPECT_EQ(toResponse(task), "Error: Overflow\n");
}

TEST(ToResponseTest, InvalidNumber)
{
    const Task task{{0, 0, Operation::Add}, 0, Status::InvalidNumber};
    EXPECT_EQ(toResponse(task), "Error: Invalid number\n");
}

TEST(ToResponseTest, Pending)
{
    const Task task{{0, 0, Operation::Add}, 0, Status::Pending};
    EXPECT_EQ(toResponse(task), "Error: The Operation is not complete\n");
}

TEST(ToResponseTest, GenericError)
{
    const Task task{{0, 0, Operation::Add}, 0, Status::Error};
    EXPECT_EQ(toResponse(task), "Error: calculation error\n");
}
} // namespace calculator::test
