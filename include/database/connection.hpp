#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include "database/db_config.hpp"

#include <memory>

namespace calculator
{
class Connection
{
  public:
    Connection();
    explicit Connection(const Config& config);
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept;
    Connection& operator=(Connection&&) noexcept;
    ~Connection();
    void* get();
    explicit operator bool() const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace calculator
#endif
