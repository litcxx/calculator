#include "database/connection.hpp"

#include <fmt/format.h>
#include <postgresql/libpq-fe.h>

#include <stdexcept>
#include <string>

namespace
{
constexpr const char* kConnect =
    "host={} port={} dbname={} user={} password={} connect_timeout=5";

struct PGconnDeleter
{
    void operator()(PGconn* conn) const
    {
        PQfinish(conn);
    }
};

using PGconnPtr = std::unique_ptr<PGconn, PGconnDeleter>;
} // namespace

namespace calculator
{
class Connection::Impl
{
  public:
    explicit Impl(const std::string& conninfo) :
        conn_(PQconnectdb(conninfo.c_str()))
    {
        if (PQstatus(conn_.get()) != CONNECTION_OK)
        {
            throw std::runtime_error(PQerrorMessage(conn_.get()));
        }
    }

    PGconn* getImpl()
    {
        return conn_.get();
    }

  private:
    PGconnPtr conn_;
};

void* Connection::get()
{
    if (impl_)
    {
        return static_cast<void*>(impl_->getImpl());
    }
    return nullptr;
}

Connection::Connection() = default;

Connection::Connection(const Config& config) :
    impl_(std::make_unique<Connection::Impl>(
        fmt::format(kConnect, config.host, config.port, config.dbname,
                    config.username, config.password)))
{}
Connection::Connection(Connection&&) noexcept = default;
Connection& Connection::operator=(Connection&&) noexcept = default;
Connection::~Connection() = default;

Connection::operator bool() const noexcept
{
    return static_cast<bool>(impl_);
}
} // namespace calculator
