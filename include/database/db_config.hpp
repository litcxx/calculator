#ifndef DB_CONFIG_HPP_
#define DB_CONFIG_HPP_

#include <stdexcept>
#include <string>

namespace calculator
{
struct Config
{
    std::string host;
    std::string port;
    std::string dbname;
    std::string username;
    std::string password;
};

inline std::string getEnvOrThrow(const char* name)
{
    const char* value = std::getenv(name); // NOLINT
    if (value == nullptr)
    {
        throw std::runtime_error(
            std::string("Environment variable not defined: ").append(name));
    }
    return value;
}

inline Config getConfig()
{
    return Config{getEnvOrThrow("DB_HOST"), getEnvOrThrow("DB_PORT"),
                  getEnvOrThrow("DB_NAME"), getEnvOrThrow("DB_USER"),
                  getEnvOrThrow("DB_PASSWORD")};
}
} // namespace calculator

#endif
