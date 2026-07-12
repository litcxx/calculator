#ifndef I_PRINTER_HPP
#define I_PRINTER_HPP

#include "utils/config.hpp"

namespace calculator
{
class IPrinter // NOLINT
{
  public:
    virtual ~IPrinter() = default;
    virtual void print(const Task& task) = 0;
};
} // namespace calculator

#endif
