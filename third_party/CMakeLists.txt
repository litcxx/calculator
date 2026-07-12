include(FetchContent)

# Fetch litcxx/litmath
FetchContent_Declare(
    litmath
    GIT_REPOSITORY https://github.com/litcxx/litmath.git
    GIT_TAG v1.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(litmath)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.0.tar.gz
)
FetchContent_MakeAvailable(nlohmann_json)

# Fetch fmtlib/fmt
FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/12.0.0.tar.gz
)
FetchContent_MakeAvailable(fmt)

# Fetch gabime/spdlog
FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.12.0.tar.gz
)
FetchContent_MakeAvailable(spdlog)

add_library(third_party INTERFACE)
target_link_libraries(third_party INTERFACE
    litmath
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
)
