#include "config.hpp"
#include "parser.hpp"

#include <gtest/gtest.h>

using namespace calculator;

struct ParserMetadata
{
    std::string line;
    CalculationRequest result;
};

class ValidParssingTest : public testing::TestWithParam<ParserMetadata>
{};

TEST_P(ValidParssingTest, Valid_user_input)
{
    // Arrange
    auto [line, result] = GetParam();

    // Act
    auto parsed_data = Parser::Parse(line);

    // Assert
    EXPECT_EQ(parsed_data.firstValue, result.firstValue);
    EXPECT_EQ(parsed_data.secondValue, result.secondValue);
    EXPECT_EQ(parsed_data.operation, result.operation);
}

INSTANTIATE_TEST_SUITE_P(
    Valid_user_input, ValidParssingTest,
    ::testing::Values(
        ParserMetadata{
            .line = R"({"first": 15, "second": 15, "operation": "add"})",
            .result = {15, 15, Operation::Add}},
        ParserMetadata{.line = R"({"first": 15, "operation": "fact"})",
                       .result = {15, 0, Operation::Fact}}));

class InvalidParssingTest : public testing::TestWithParam<std::string_view>
{};

TEST_P(InvalidParssingTest, Invalid_user_input)
{
    // Arrange
    auto line = GetParam();

    // Act & Assert
    EXPECT_ANY_THROW(Parser::Parse(line));
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
