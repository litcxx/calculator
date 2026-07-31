#ifndef RESPONSE_HPP_
#define RESPONSE_HPP_

#include "utils/config.hpp"

#include <string>

namespace calculator
{
// Renders a completed Task into a newline-terminated response line.
std::string toResponse(const Task& task);
} // namespace calculator

#endif
