#ifndef I_REPOSITORE_HPP_
#define I_REPOSITORE_HPP_

#include "utils/config.hpp"

#include <optional>

namespace calculator
{
class IRepository // NOLINT
{
  public:
    virtual ~IRepository() = default;
    virtual std::optional<Task> get(const CalculationRequest& request) = 0;
    virtual void save(const Task& task) = 0;
};
} // namespace calculator

#endif
