#include "config.hpp"

#include <gtest/gtest.h>

namespace
{
class TestEnvironment : public ::testing::Environment
{
  public:
    void SetUp() override
    {
        calculator::test::TestConfig::
            config(); // падает ТОЛЬКО при запуске тестов
    }
};
} // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment); // NOLINT

    return RUN_ALL_TESTS();
}
