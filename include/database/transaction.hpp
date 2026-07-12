#ifndef TRANSACTION_HPP_
#define TRANSACTION_HPP_

#include "database/connection_guard.hpp"
#include "utils/config.hpp"

#include <functional>
#include <optional>

namespace calculator
{
class Transaction
{
  public:
    explicit Transaction(ConnectionGuard conn);
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction(Transaction&& rhs) noexcept;
    Transaction& operator=(Transaction&& rhs) noexcept;
    ~Transaction();

    std::optional<Task> fetch(const CalculationRequest& request);
    void fetchAll(const std::function<void(const Task& task)>& func);
    void record(const Task& task);
    void commit();

    friend void swap(Transaction& lhs, Transaction& rhs) noexcept;

  private:
    void exec(const char* command);

    ConnectionGuard conn_;
    bool active_;
};
} // namespace calculator

#endif
