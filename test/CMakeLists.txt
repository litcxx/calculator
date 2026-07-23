include(FetchContent)

# Fetch gtest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.tar.gz
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(UNIT_SOURCES
    unit/parser.cpp
    unit/cache.cpp
    unit/calculator.cpp
    unit/response.cpp
    unit/request_handler.cpp
)

set(INTEGRATION_SOURCES
    integration/connection.cpp
    integration/transaction.cpp
    integration/connection_guard.cpp
    integration/connection_pool.cpp
    integration/main.cpp
)

set(INTEGRATION_COMMIT_SOURCES
    integration/repository.cpp
    integration/main.cpp
)

add_executable(unit_tests ${UNIT_SOURCES})

target_link_libraries(unit_tests PRIVATE
    coverage_flags
    calc_core
    calc_warnings
    gtest
    gtest_main
    gmock
)

find_package(PostgreSQL REQUIRED)

add_executable(integration_tests ${INTEGRATION_SOURCES})

target_link_libraries(integration_tests PRIVATE
    coverage_flags
    calc_core
    calc_warnings
    gtest
    PRIVATE PostgreSQL::PostgreSQL
    PRIVATE third_party
)

add_executable(commit_tests ${INTEGRATION_COMMIT_SOURCES})
target_link_libraries(commit_tests PRIVATE
    coverage_flags
    calc_core
    calc_warnings
    gtest
    PRIVATE PostgreSQL::PostgreSQL
    PRIVATE third_party
)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

add_executable(network_tests integration/network.cpp)
target_link_libraries(network_tests PRIVATE
    coverage_flags
    calc_core
    calc_warnings
    gtest
    gtest_main
    Boost::headers
    Threads::Threads
)


include(GoogleTest)

gtest_discover_tests(unit_tests
    PROPERTIES LABELS "unit"
)

gtest_discover_tests(integration_tests
    PROPERTIES LABELS "integration"
)

gtest_discover_tests(commit_tests
    PROPERTIES LABELS "commit"
)

gtest_discover_tests(network_tests
    PROPERTIES LABELS "network" TIMEOUT 30
)

# --- VALGRIND ---
if(USE_VALGRIND)
    find_program(VALGRIND_BIN NAMES valgrind)

    if(VALGRIND_BIN)
        set(VALGRIND_OPTS
            --leak-check=full
            --show-leak-kinds=all
            --track-origins=yes
            --error-exitcode=1
            --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/valgrind.supp
        )

        add_custom_target(valgrind-unit
            COMMAND ${VALGRIND_BIN} ${VALGRIND_OPTS} $<TARGET_FILE:unit_tests>
            DEPENDS unit_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running unit tests with Valgrind..."
        )

        add_custom_target(valgrind-integration
            COMMAND ${VALGRIND_BIN} ${VALGRIND_OPTS} $<TARGET_FILE:integration_tests>
            DEPENDS integration_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running integration tests with Valgrind..."
        )

        add_custom_target(valgrind-commit
            COMMAND ${VALGRIND_BIN} ${VALGRIND_OPTS} $<TARGET_FILE:commit_tests>
            DEPENDS commit_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running integration tests with Valgrind..."
        )
    else()
        message(WARNING "valgrind is not found: ${VALGRIND_BIN}")
    endif()
endif()
