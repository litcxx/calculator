include(FetchContent)

# Fetch litcxx/litmath
FetchContent_Declare(
    litmath
    GIT_REPOSITORY https://github.com/litcxx/litmath.git
    GIT_TAG v1.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(litmath)
# target_link_libraries(${PROJECT_NAME} PRIVATE litmath)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.0.tar.gz
)
FetchContent_MakeAvailable(nlohmann_json)
# target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json)

# Fetch gabime/spdlog
FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.15.0.tar.gz
)
FetchContent_MakeAvailable(spdlog)
# target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)

add_library(third_party INTERFACE)
target_link_libraries(third_party INTERFACE
    litmath
    nlohmann_json
    spdlog
)
