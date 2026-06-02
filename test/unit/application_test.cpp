#include "application.hpp"

#include <gtest/gtest.h>

using namespace calculator;

TEST(ApplicationTest, Successfull_addition)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"first": 2, "second": 3, "operation": "add"})");
    std::vector<char*> argv = {name.data(), format.data()};
    int argc = 2;
    Application sut;

    // Act
    sut.run(argc, argv.data());

    // Assert
    EXPECT_EQ(sut.getTask().status, Status::Success);
    EXPECT_EQ(sut.getTask().result, 5);
}

TEST(ApplicationTest, Successfull_factorial)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"first": 10, "operation": "fact"})");
    std::vector<char*> argv = {name.data(), format.data()};
    int argc = 2;
    Application sut;

    // Act
    sut.run(argc, argv.data());

    // Assert
    EXPECT_EQ(sut.getTask().status, Status::Success);
    EXPECT_EQ(sut.getTask().result, 3628800);
}

TEST(ApplicationTest, Invalid_json_format)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"invalid": "format"})");
    std::vector<char*> argv = {name.data(), format.data()};
    int argc = 2;
    Application sut;

    // Act & Arrange
    EXPECT_THROW(sut.run(argc, argv.data()), std::logic_error);
    EXPECT_EQ(sut.getTask().status, Status::Error);
}

TEST(ApplicationTest, Division_by_zero)
{
    // Arrange
    std::string name("calc");
    std::string format(R"({"first": 10, "second": 0, "operation": "div"})");
    std::vector<char*> argv = {name.data(), format.data()};
    int argc = 2;
    Application sut;

    // Act & Arrange
    EXPECT_THROW(sut.run(argc, argv.data()), std::logic_error);
    EXPECT_EQ(sut.getTask().status, Status::Error);
}

TEST(ApplicationTest, Invalid_arguments_count)
{
    // Arrange
    std::string name("calc");
    std::vector<char*> argv = {name.data()};
    int argc = 1;
    Application sut;

    // Act & Arrange
    EXPECT_THROW(sut.run(argc, argv.data()), std::logic_error);
    EXPECT_EQ(sut.getTask().status, Status::Error);
}
