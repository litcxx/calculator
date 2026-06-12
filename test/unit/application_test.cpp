#include "application.hpp"

#include <gtest/gtest.h>

TEST(ApplicationTest, SuccessfullAddition)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"first": 2, "second": 3, "operation": "add"})");
    std::vector<char*> argv = {name.data(), format.data()};
    const int argc = 2;
    calculator::Application sut;

    // Act
    sut.run(argc, argv.data());

    // Assert
    EXPECT_EQ(sut.getTask().status, calculator::Status::Success);
    EXPECT_EQ(sut.getTask().result, 5);
}

TEST(ApplicationTest, SuccessfullFactorial)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"first": 10, "operation": "fact"})");
    std::vector<char*> argv = {name.data(), format.data()};
    const int argc = 2;
    calculator::Application sut;

    // Act
    sut.run(argc, argv.data());

    // Assert
    EXPECT_EQ(sut.getTask().status, calculator::Status::Success);
    EXPECT_EQ(sut.getTask().result, 3628800);
}

TEST(ApplicationTest, InvalidJsonFormat)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"invalid": "format"})");
    std::vector<char*> argv = {name.data(), format.data()};
    const int argc = 2;
    calculator::Application sut;

    // Act & Arrange
    EXPECT_THROW(sut.run(argc, argv.data()), std::logic_error);
    EXPECT_EQ(sut.getTask().status, calculator::Status::Error);
}

TEST(ApplicationTest, DivisionByZero)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"first": 10, "second": 0, "operation": "div"})");
    std::vector<char*> argv = {name.data(), format.data()};
    const int argc = 2;
    calculator::Application sut;

    // Act & Arrange
    EXPECT_THROW(sut.run(argc, argv.data()), std::logic_error);
    EXPECT_EQ(sut.getTask().status, calculator::Status::Error);
}

TEST(ApplicationTest, InvalidArgumentsCount)
{
    // Arrange
    std::string name("calc");
    std::vector<char*> argv = {name.data()};
    const int argc = 1;
    calculator::Application sut;

    // Act & Arrange
    EXPECT_THROW(sut.run(argc, argv.data()), std::logic_error);
    EXPECT_EQ(sut.getTask().status, calculator::Status::Error);
}
