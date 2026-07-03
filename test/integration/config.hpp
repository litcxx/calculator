#include "database/db_config.hpp"

using namespace calculator; // NOLINT

namespace calculator::test
{
class TestConfig
{
  public:
    static Config& config()
    {
        static Config cfg = getConfig();
        return cfg;
    }
};
} // namespace calculator::test
