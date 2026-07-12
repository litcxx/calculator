#include "core/i_printer.hpp"
#include "utils/config.hpp"

#include <iostream>

namespace calculator
{
class StdoutPrinter : public IPrinter
{
  public:
    void print(const Task& task) override
    {
        switch (task.status)
        {
            case Status::Success:
                std::cout << "Success : " << task.result << '\n';
                break;
            case Status::Pending:
                std::cout << "Error: The Operation is not complete\n";
                break;
            case Status::Overflow:
                std::cout << "Error: Overflow\n";
                break;
            case Status::DivideByZero:
                std::cout << "Error: Divide by zero\n";
                break;
            case Status::InvalidNumber:
                std::cout << "Error: Invalid number\n";
                break;
            case Status::Error:
                std::cout << "Error: calculation error\n";
                break;
            default:
                static_assert(true);
        }
    }
};
} // namespace calculator
