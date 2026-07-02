#ifndef REPOSITORY_HPP_
#define REPOSITORY_HPP_

#include "core/i_repository.hpp"
#include "storage/cache.hpp"
#include "storage/connection_pool.hpp"

namespace calculator
{
class Repository : public IRepository
{
  public:
    explicit Repository(ConnectionPool pool, Cache cache);

    std::optional<Task> get(const CalculationRequest& request) override;
    void save(const Task& task) override;

  private:
    ConnectionPool pool_;
    Cache cache_;
};
} // namespace calculator

#endif
