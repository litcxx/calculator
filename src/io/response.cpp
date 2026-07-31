#include "io/response.hpp"

#include <string>

namespace calculator
{
std::string toResponse(const Task& task)
{
    switch (task.status)
    {
        case Status::Success:
            return "Success : " + std::to_string(task.result) + "\n";
        case Status::Pending:
            return "Error: The Operation is not complete\n";
        case Status::Overflow:
            return "Error: Overflow\n";
        case Status::DivideByZero:
            return "Error: Divide by zero\n";
        case Status::InvalidNumber:
            return "Error: Invalid number\n";
        case Status::Error:
            return "Error: calculation error\n";
    }
    return "Error: calculation error\n";
}
} // namespace calculator
