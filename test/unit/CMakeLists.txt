include(FetchContent)

# Fetch gtest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.tar.gz
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SOURCES
    parser_test.cpp
    application_test.cpp
)

add_executable(unit_tests ${SOURCES})

target_link_libraries(unit_tests PRIVATE
    calc_core
    calc_warnings
    gtest
    gtest_main
    gmock
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
