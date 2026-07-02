#include "storage/cache.hpp"

#include "utils/config.hpp"

#include <optional>
#include <unordered_map>

namespace calculator
{
std::optional<Task> Cache::get(const CalculationRequest& request)
{
    auto result = map_.find(request);
    if (result != map_.end())
    {
        return result->second;
    }

    if (request.operation == Operation::Add ||
        request.operation == Operation::Mul)
    {
        const CalculationRequest reversedRequest{
            request.secondValue, request.firstValue, request.operation};
        result = map_.find(reversedRequest);
        if (result != map_.end())
        {
            return result->second;
        }
    }

    return std::nullopt;
}
void Cache::insert(const Task& task)
{
    map_[task.request] = task;
}
} // namespace calculator
