#include "database/transaction.hpp"

#include "database/connection_guard.hpp"
#include "utils/logger.hpp"

#include <fmt/format.h>
#include <postgresql/libpq-fe.h>

#include <array>
#include <cassert>
#include <charconv>
#include <stdexcept>
#include <utility>

namespace
{
constexpr const char* kInsertQuery =
    "INSERT INTO tasks (first_value, second_value, operation, result, "
    "status) VALUES ($1, $2, $3, $4, $5);";

constexpr const char* kFetchQuery =
    "SELECT result, status FROM tasks WHERE first_value=$1 AND "
    "second_value=$2 AND operation=$3";

constexpr const char* kFetchAllData =
    "SELECT first_value, second_value, operation, result, status FROM tasks";

constexpr const char* kFetchQueryAddMul =
    "SELECT result, status FROM tasks WHERE (first_value=$1 AND "
    "second_value=$2 OR first_value=$2 AND second_value=$1) AND operation=$3";

constexpr const char* kBeginTransaction =
    "BEGIN TRANSACTION ISOLATION LEVEL READ COMMITTED;";
constexpr const char* kCommitTransaction = "COMMIT;";
constexpr const char* kRollbackTransaction = "ROLLBACK;";

struct PGresultDeleter
{
    void operator()(PGresult* res) const
    {
        PQclear(res);
    }
};

using PGresultPtr = std::unique_ptr<PGresult, PGresultDeleter>;

int toInt(std::string_view line)
{
    int value{};
    auto [_, ec] =
        std::from_chars(line.data(), line.data() + line.size(), // NOLINT
                        value);
    if (ec != std::errc())
    {
        throw std::runtime_error("Number conversion error");
    }
    return value;
}
} // namespace

namespace calculator
{
Transaction::Transaction(ConnectionGuard conn) :
    conn_{std::move(conn)}, active_{false}
{
    exec(kBeginTransaction);
    active_ = true; // NOLINT
}

Transaction::Transaction(Transaction&& rhs) noexcept :
    conn_{std::move(rhs.conn_)}, active_{std::exchange(rhs.active_, false)}
{}

Transaction& Transaction::operator=(Transaction&& rhs) noexcept
{
    Transaction tmp(std::move(rhs));
    swap(*this, tmp);
    return *this;
}

void swap(Transaction& lhs, Transaction& rhs) noexcept
{
    using std::swap;
    swap(lhs.conn_, rhs.conn_);
    swap(lhs.active_, rhs.active_);
}

Transaction::~Transaction()
{
    if (active_)
    {
        try
        {
            exec(kRollbackTransaction);
        }
        catch (...) // NOLINT
        {
            Logger::getInstance().error("Rollback transaction error");
        }
    }
}

std::optional<Task> Transaction::fetch(const CalculationRequest& request)
{
    assert(active_);

    auto* connPtr = static_cast<PGconn*>(conn_.get());

    const std::string firstValue = fmt::to_string(request.firstValue);
    const std::string secondValue = fmt::to_string(request.secondValue);
    const std::string operation =
        fmt::to_string(static_cast<int>(request.operation));

    const std::array<const char*, 3> params = {
        firstValue.c_str(), secondValue.c_str(), operation.c_str()};

    const char* query = (request.operation == Operation::Add ||
                         request.operation == Operation::Mul)
                            ? kFetchQueryAddMul
                            : kFetchQuery;

    const PGresultPtr res(PQexecParams(connPtr, query, 3, nullptr,
                                       params.data(), nullptr, nullptr, 0));

    auto* resPtr = res.get();

    if (resPtr == nullptr)
    {
        throw std::runtime_error(
            fmt::format("PQexecParams: {}", PQerrorMessage(connPtr)));
    }

    if (PQresultStatus(resPtr) != PGRES_TUPLES_OK)
    {
        throw std::runtime_error(fmt::format("Select query error: {}",
                                             PQresultErrorMessage(res.get())));
    }

    if (PQntuples(resPtr) == 0)
    {
        return std::nullopt;
    }

    assert(PQnfields(res.get()) == 2);

    const char* result = PQgetvalue(res.get(), 0, 0);
    const char* status = PQgetvalue(res.get(), 0, 1);

    return Task{request, toInt(result), static_cast<Status>(toInt(status))};
}

void Transaction::fetchAll(const std::function<void(const Task& task)>& func)
{
    assert(active_);
    Logger::getInstance().info("fetch all data");

    auto* connPtr = static_cast<PGconn*>(conn_.get());
    const PGresultPtr res(PQexec(connPtr, kFetchAllData));
    auto* resPtr = res.get();

    if (resPtr == nullptr)
    {
        throw std::runtime_error(
            fmt::format("PQexecParams: {}", PQerrorMessage(connPtr)));
    }

    if (PQresultStatus(resPtr) != PGRES_TUPLES_OK)
    {
        throw std::runtime_error(fmt::format("Select query error: {}",
                                             PQresultErrorMessage(resPtr)));
    }

    const int nrows = PQntuples(resPtr);
    assert(PQnfields(res.get()) == 5);

    for (int i = 0; i < nrows; ++i)
    {
        const char* firstValue = PQgetvalue(resPtr, i, 0);
        const char* secondValue = PQgetvalue(resPtr, i, 1);
        const char* operation = PQgetvalue(resPtr, i, 2);
        const char* result = PQgetvalue(resPtr, i, 3);
        const char* status = PQgetvalue(resPtr, i, 4);

        const Task task{{toInt(firstValue), toInt(secondValue),
                         static_cast<Operation>(toInt(operation))},
                        toInt(result),
                        static_cast<Status>(toInt(status))};

        Logger::getInstance().info("insert task to func");
        func(task);
    }
}

void Transaction::record(const Task& task)
{
    assert(active_);

    auto* connPtr = static_cast<PGconn*>(conn_.get());

    const std::string firstValue = fmt::to_string(task.request.firstValue);
    const std::string secondValue = fmt::to_string(task.request.secondValue);
    const std::string operation =
        fmt::to_string(static_cast<int>(task.request.operation));
    const std::string result = fmt::to_string(task.result);
    const std::string status = fmt::to_string(static_cast<int>(task.status));

    const std::array<const char*, 5> params = {
        firstValue.c_str(), secondValue.c_str(), operation.c_str(),
        result.c_str(), status.c_str()};

    const PGresultPtr res(PQexecParams(connPtr, kInsertQuery, 5, nullptr,
                                       params.data(), nullptr, nullptr, 0));

    auto* resPtr = res.get();

    if (resPtr == nullptr)
    {
        throw std::runtime_error(
            fmt::format("PQexecParams: {}", PQerrorMessage(connPtr)));
    }

    if (PQresultStatus(resPtr) != PGRES_COMMAND_OK)
    {
        throw std::runtime_error(fmt::format("Insert query error: {}",
                                             PQresultErrorMessage(resPtr)));
    }
}

void Transaction::commit()
{
    assert(active_);

    exec(kCommitTransaction);
    active_ = false;
}

void Transaction::exec(const char* command)
{
    auto* connPtr = static_cast<PGconn*>(conn_.get());
    const PGresultPtr res(PQexec(connPtr, command));
    auto* resPtr = res.get();

    if (resPtr == nullptr)
    {
        throw std::runtime_error(fmt::format("Command: {} error: {}", command,
                                             PQerrorMessage(connPtr)));
    }

    if (PQresultStatus(resPtr) != PGRES_COMMAND_OK)
    {
        throw std::runtime_error(fmt::format("Command: {} error: {}", command,
                                             PQresultErrorMessage(resPtr)));
    }
}

} // namespace calculator
