#include "database/connection.hpp"

namespace calculator::test
{
inline const Config kDBValidConfig = {"localhost", "5433", "test_calculatordb",
                                      "user123", "user123"};

inline const Config kDBInvalidValidConfig = {"foo_host", "foo_port", "foo_db",
                                             "foo_user", "foo_password"};
} // namespace calculator::test
