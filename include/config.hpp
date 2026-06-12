#ifndef CONFIG_H_
#define CONFIG_H_

#include <cstdint>

namespace calculator
{
enum class Operation : std::uint8_t
{
    Add,
    Sub,
    Mul,
    Div,
    Pow,
    Fact,
};

enum class Status : std::uint8_t
{
    Pending,
    Success,
    Error,
};

struct CalculationRequest
{
    int firstValue{};
    int secondValue{};
    Operation operation{};
};

struct Task
{
    CalculationRequest request;
    int result{};
    Status status{Status::Pending};
};
} // namespace calculator

#endif
