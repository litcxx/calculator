#ifndef CACHE_HPP_
#define CACHE_HPP_

#include "utils/config.hpp"

#include <cstddef>
#include <optional>
#include <unordered_map>

namespace calculator
{
class Cache
{
  public:
    Cache() = default;
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    Cache(Cache&&) noexcept = default;
    Cache& operator=(Cache&&) noexcept = default;
    ~Cache() = default;
    std::optional<Task> get(const CalculationRequest& request);
    void insert(const Task& task);

  private:
    struct Hash
    {
        std::size_t operator()(const CalculationRequest& request) const
        {
            const std::size_t hash1 = std::hash<int>{}(request.firstValue);
            const std::size_t hash2 = std::hash<int>{}(request.secondValue);
            const std::size_t hash3 =
                std::hash<int>{}(static_cast<int>(request.operation));
            return hash1 ^ (hash2 << 1) ^ (hash3 << 2);
        }
    };
    std::unordered_map<CalculationRequest, Task, Hash> map_;
};
} // namespace calculator

#endif
