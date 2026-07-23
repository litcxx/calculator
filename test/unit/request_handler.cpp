#include "app/request_handler.hpp"

#include "core/i_calculator.hpp"
#include "core/i_parser.hpp"
#include "core/i_repository.hpp"
#include "utils/config.hpp"

#include <memory>
#include <optional>
#include <stdexcept>

#include "gmock/gmock.h"
#include <gtest/gtest.h>

namespace calculator::test
{
namespace
{
class MockRepository : public IRepository
{
  public:
    MOCK_METHOD(std::optional<Task>, get, (const CalculationRequest& request),
                (override));
    MOCK_METHOD(void, save, (const Task& task), (override));
};

class MockParser : public IParser
{
  public:
    MOCK_METHOD(void, validateInput, (int argc, char** argv), (override));
    MOCK_METHOD(CalculationRequest, parse, (std::string_view line), (override));
};

class MockCalculator : public ICalculator
{
  public:
    MOCK_METHOD(Task, calculate, (const CalculationRequest& request),
                (override));
};
} // namespace

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

TEST(RequestHandlerTest, CacheHitReturnsStoredResultWithoutCalculating)
{
    const CalculationRequest request{2, 3, Operation::Add};
    const Task task{request, 5, Status::Success};
    auto repository = std::make_unique<MockRepository>();
    auto parser = std::make_unique<MockParser>();
    auto calculator = std::make_unique<MockCalculator>();
    MockRepository& mockRepo = *repository;
    MockParser& mockParser = *parser;
    MockCalculator& mockCalc = *calculator;

    EXPECT_CALL(mockParser, parse(_)).WillOnce(Return(request));
    EXPECT_CALL(mockRepo, get(request))
        .WillOnce(Return(std::optional<Task>{task}));
    EXPECT_CALL(mockCalc, calculate(_)).Times(0);
    EXPECT_CALL(mockRepo, save(_)).Times(0);

    RequestHandler handler(std::move(repository), std::move(parser),
                           std::move(calculator));
    EXPECT_EQ(handler.handle(R"({"first":2,"second":3,"operation":"add"})"),
              "Success : 5\n");
}

TEST(RequestHandlerTest, CacheMissCalculatesAndSaves)
{
    const CalculationRequest request{2, 3, Operation::Add};
    const Task task{request, 5, Status::Success};
    auto repository = std::make_unique<MockRepository>();
    auto parser = std::make_unique<MockParser>();
    auto calculator = std::make_unique<MockCalculator>();
    MockRepository& mockRepo = *repository;
    MockParser& mockParser = *parser;
    MockCalculator& mockCalc = *calculator;

    EXPECT_CALL(mockParser, parse(_)).WillOnce(Return(request));
    EXPECT_CALL(mockRepo, get(request)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockCalc, calculate(request)).WillOnce(Return(task));
    EXPECT_CALL(mockRepo, save(task)).Times(1);

    RequestHandler handler(std::move(repository), std::move(parser),
                           std::move(calculator));
    EXPECT_EQ(handler.handle(R"({"first":2,"second":3,"operation":"add"})"),
              "Success : 5\n");
}

TEST(RequestHandlerTest, InvalidRequestReturnsErrorLineAndDoesNotTouchRepository)
{
    auto repository = std::make_unique<MockRepository>();
    auto parser = std::make_unique<MockParser>();
    auto calculator = std::make_unique<MockCalculator>();
    MockRepository& mockRepo = *repository;
    MockParser& mockParser = *parser;

    EXPECT_CALL(mockParser, parse(_)).WillOnce(Throw(std::logic_error("bad json")));
    EXPECT_CALL(mockRepo, get(_)).Times(0);

    RequestHandler handler(std::move(repository), std::move(parser),
                           std::move(calculator));
    const std::string response = handler.handle("garbage");
    EXPECT_THAT(response, ::testing::HasSubstr("Error"));
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response.back(), '\n'); // newline-terminated: frames on the wire
}
} // namespace calculator::test
