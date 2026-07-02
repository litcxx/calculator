#include "io/parser.hpp"

#include "utils/config.hpp"

#include <ostream>

#include <gtest/gtest.h>

using namespace calculator; // NOLINT

namespace
{
struct ParserMetadata
{
    std::string line;
    CalculationRequest result;
};

class ValidParssingTest : public testing::TestWithParam<ParserMetadata>
{};

class InvalidParssingTest : public testing::TestWithParam<std::string_view>
{};

void PrintTo(const ParserMetadata& metadata, std::ostream* os) // NOLINT
{
    *os << "{ field1: " << metadata.line << '\n';
}
} // namespace

TEST_P(ValidParssingTest, ValidUserInput)
{
    // Arrange
    const auto [line, result] = GetParam();
    Parser sut;

    // Act
    const auto parsedData = sut.parse(line);

    // Assert
    EXPECT_EQ(parsedData.firstValue, result.firstValue);
    EXPECT_EQ(parsedData.secondValue, result.secondValue);
    EXPECT_EQ(parsedData.operation, result.operation);
}

INSTANTIATE_TEST_SUITE_P(
    Valid_user_input, ValidParssingTest,
    ::testing::Values(
        ParserMetadata{R"({"first": 15, "second": 15, "operation": "add"})",
                       {15, 15, Operation::Add}},
        ParserMetadata{R"({"first": 15, "operation": "fact"})",
                       {15, 0, Operation::Fact}}));

TEST_P(InvalidParssingTest, InvalidUserInput)
{
    // Arrange
    const auto line = GetParam();
    Parser sut;

    // Act & Assert
    EXPECT_ANY_THROW(sut.parse(line));
}

INSTANTIATE_TEST_SUITE_P(
    Invalid_json_format, InvalidParssingTest,
    ::testing::Values(R"({"firs": 15, "second": 15, "operation": "add"})",
                      R"({"first": 15, "secon": 15, "operation": "add"})",
                      R"({"first": 15, "second": 15, "operatio": "add"})"));

INSTANTIATE_TEST_SUITE_P(
    Invalid_calculation_request_format, InvalidParssingTest,
    ::testing::Values(R"({"first": 15, "operation": "add"})",
                      R"({"second": 15, "operation": "sub"})",
                      R"({"first": 15, "operation": "sub"})",
                      R"({"second": 15, "operation": "sub"})",
                      R"({"first": 15, "operation": "mul"})",
                      R"({"second": 15, "operation": "mul"})",
                      R"({"first": 15, "operation": "div"})",
                      R"({"second": 15, "operation": "div"})",
                      R"({"first": 15, "operation": "sub"})",
                      R"({"second": 15, "operation": "sub"})",
                      R"({"first": 15, "operation": "pow"})",
                      R"({"second": 15, "operation": "pow"})",
                      R"({"first": 15, "second": 15, "operation": "fact"})",
                      R"({"second": 15, "operation": "fact"})"));

INSTANTIATE_TEST_SUITE_P(
    Invalid_operation, InvalidParssingTest,
    ::testing::Values(R"({"first": 15, "second": 15, "operation": "foo"})"));

TEST(ValidateInputTest, IncorrectNumberOfParameters)
{
    // Arrange
    const int argc = 1;
    char arg0[] = "name";  // NOLINT
    char* argv[] = {arg0}; // NOLINT
    Parser sut;

    // Act & Assert
    EXPECT_THROW(sut.validateInput(argc, argv), std::logic_error); // NOLINT
}

TEST(ValidateInputTest, EmptyString)
{
    // Arrange
    const int argc = 2;
    Parser sut;

    // Act & Assert
    EXPECT_THROW(sut.validateInput(argc, nullptr), std::logic_error);
}

TEST(ValidateInputTest, FullHelpInput)
{
    // Arrange
    const int argc = 2;
    char arg0[] = "name";        // NOLINT
    char arg1[] = "--help";      // NOLINT
    char* argv[] = {arg0, arg1}; // NOLINT
    Parser sut;

    // Act & Assert
    EXPECT_THROW(sut.validateInput(argc, argv), std::string); // NOLINT
}

TEST(ValidateInputTest, ShortHelpInput)
{
    // Arrange
    const int argc = 2;
    char arg0[] = "name";        // NOLINT
    char arg1[] = "-h";          // NOLINT
    char* argv[] = {arg0, arg1}; // NOLINT
    Parser sut;

    // Act & Assert
    EXPECT_THROW(sut.validateInput(argc, argv), std::string); // NOLINT
}

TEST(ValidateInputTest, ValidInput)
{
    // Arrange
    const int argc = 2;
    char arg0[] = "name";        // NOLINT
    char arg1[] = "name";        // NOLINT
    char* argv[] = {arg0, arg1}; // NOLINT
    Parser sut;

    // Act & Assert
    EXPECT_NO_THROW(sut.validateInput(argc, argv)); // NOLINT
}
