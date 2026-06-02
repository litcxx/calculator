#include "config.hpp"
#include "parser.hpp"

#include <gtest/gtest.h>

namespace
{
struct ParserMetadata
{
    std::string line;
    calculator::CalculationRequest result;
};

class ValidParssingTest : public testing::TestWithParam<ParserMetadata>
{};

class InvalidParssingTest : public testing::TestWithParam<std::string_view>
{};
} // namespace

TEST_P(ValidParssingTest, ValidUserInput)
{
    // Arrange
    auto [line, result] = GetParam();

    // Act
    auto parsedData = calculator::Parser::parse(line);

    // Assert
    EXPECT_EQ(parsedData.firstValue, result.firstValue);
    EXPECT_EQ(parsedData.secondValue, result.secondValue);
    EXPECT_EQ(parsedData.operation, result.operation);
}

INSTANTIATE_TEST_SUITE_P(
    Valid_user_input, ValidParssingTest,
    ::testing::Values(
        ParserMetadata{
            .line = R"({"first": 15, "second": 15, "operation": "add"})",
            .result = {15, 15, calculator::Operation::Add}},
        ParserMetadata{.line = R"({"first": 15, "operation": "fact"})",
                       .result = {15, 0, calculator::Operation::Fact}}));

TEST_P(InvalidParssingTest, InvalidUserInput)
{
    // Arrange
    auto line = GetParam();

    // Act & Assert
    EXPECT_ANY_THROW(calculator::Parser::parse(line));
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
