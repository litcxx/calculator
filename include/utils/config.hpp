#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstdint>

namespace calculator
{
enum class Operation : std::uint8_t
{
    Add = 0x00,
    Sub = 0x01,
    Mul = 0x02,
    Div = 0x03,
    Pow = 0x04,
    Fact = 0x05,
};

enum class Status : std::uint8_t
{
    Success = 0x00,
    Pending = 0x01,
    Overflow = 0x02,
    DivideByZero = 0x03,
    InvalidNumber = 0x04,
    Error = 0x05,
};

struct CalculationRequest
{
    int firstValue{};
    int secondValue{};
    Operation operation{};
};

inline bool operator==(const CalculationRequest& lhs,
                       const CalculationRequest& rhs)
{
    return lhs.firstValue == rhs.firstValue &&
           lhs.secondValue == rhs.secondValue && lhs.operation == rhs.operation;
}

struct Task
{
    CalculationRequest request;
    int result{};
    Status status{Status::Pending};
};

inline bool operator==(const Task& lhs, const Task& rhs)
{
    return lhs.request == rhs.request && lhs.result == rhs.result &&
           lhs.status == rhs.status;
}
} // namespace calculator

#endif
