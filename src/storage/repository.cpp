#include "storage/repository.hpp"

#include "database/connection_guard.hpp"

#include <database/transaction.hpp>
#include <storage/cache.hpp>
#include <storage/connection_pool.hpp>

#include <utility>

namespace calculator
{
Repository::Repository(ConnectionPool pool, Cache cache) :
    pool_{std::move(pool)}, cache_{std::move(cache)}
{
    // WARMUP CACHE
    Transaction transaction(pool_.acquire());
    transaction.fetchAll(
        [this](const Task& task) { this->cache_.insert(task); });
}

std::optional<Task> Repository::get(const CalculationRequest& request)
{
    return cache_.get(request);
}

void Repository::save(const Task& task)
{
    Transaction transaction(pool_.acquire());
    transaction.record(task);
    transaction.commit();

    cache_.insert(task);
}
} // namespace calculator
