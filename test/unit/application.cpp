#include "app/application.hpp"

#include "core/i_calculator.hpp"
#include "core/i_parser.hpp"
#include "core/i_printer.hpp"
#include "core/i_repository.hpp"
#include "utils/config.hpp"

#include <memory>

#include "gmock/gmock.h"
#include <gtest/gtest.h>

using namespace calculator; // NOLINT

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

class MockPrinter : public IPrinter
{
  public:
    MOCK_METHOD(void, print, (const Task& task), (override));
};

} // namespace

TEST(ApplicationTest, RunCacheHit)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    const std::optional<Task> retTask{task};
    auto repository = std::make_unique<MockRepository>();
    MockRepository& mockRepository = *repository;
    auto parser = std::make_unique<MockParser>();
    MockParser& mockParser = *parser;
    auto calculator = std::make_unique<MockCalculator>();
    MockCalculator& mockCalculator = *calculator;
    auto printer = std::make_unique<MockPrinter>();
    MockPrinter& mockPrinter = *printer;
    const int argc = 2;
    char arg0[] = "name";        // NOLINT
    char arg1[] = "-h";          // NOLINT
    char* argv[] = {arg0, arg1}; // NOLINT
    Application sut(std::move(repository), std::move(parser),
                    std::move(calculator), std::move(printer));

    // Act & Assert
    EXPECT_CALL(mockParser, validateInput(::testing::_, ::testing::_)).Times(1);
    EXPECT_CALL(mockParser, parse(::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(request));
    EXPECT_CALL(mockRepository, get(request))
        .Times(1)
        .WillOnce(::testing::Return(retTask));
    EXPECT_CALL(mockPrinter, print(task)).Times(1);
    EXPECT_CALL(mockCalculator, calculate(request)).Times(0);
    EXPECT_CALL(mockRepository, save(task)).Times(0);
    sut.run(argc, argv); // NOLINT
}

TEST(ApplicationTest, RunCacheMiss)
{
    // Arrange
    const CalculationRequest request{1, 1, Operation::Add};
    const Task task{request, 2, Status::Success};
    auto repository = std::make_unique<MockRepository>();
    MockRepository& mockRepository = *repository;
    auto parser = std::make_unique<MockParser>();
    MockParser& mockParser = *parser;
    auto calculator = std::make_unique<MockCalculator>();
    MockCalculator& mockCalculator = *calculator;
    auto printer = std::make_unique<MockPrinter>();
    MockPrinter& mockPrinter = *printer;
    const int argc = 2;
    char arg0[] = "name";        // NOLINT
    char arg1[] = "-h";          // NOLINT
    char* argv[] = {arg0, arg1}; // NOLINT
    Application sut(std::move(repository), std::move(parser),
                    std::move(calculator), std::move(printer));

    // Act & Assert
    EXPECT_CALL(mockParser, validateInput(::testing::_, ::testing::_)).Times(1);
    EXPECT_CALL(mockParser, parse(::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(request));
    EXPECT_CALL(mockRepository, get(request))
        .Times(1)
        .WillOnce(::testing::Return(std::nullopt));
    EXPECT_CALL(mockPrinter, print(task)).Times(1);
    EXPECT_CALL(mockCalculator, calculate(request))
        .Times(1)
        .WillOnce(::testing::Return(task));
    EXPECT_CALL(mockRepository, save(task)).Times(1);
    sut.run(argc, argv); // NOLINT
}
