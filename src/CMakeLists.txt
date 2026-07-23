set(SOURCES
    app/application.cpp
    app/calculator.cpp
    database/connection.cpp
    database/connection_guard.cpp
    database/transaction.cpp
    io/parser.cpp
    storage/repository.cpp
    storage/cache.cpp
    storage/connection_pool.cpp
    utils/logger.cpp
    utils/signal_handler.cpp
)

find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

add_library(calc_core STATIC
    ${SOURCES}
)

target_include_directories(calc_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(calc_core
    PRIVATE third_party
    PUBLIC calc_warnings
    PRIVATE PostgreSQL::PostgreSQL
    PUBLIC Threads::Threads
    PRIVATE coverage_flags
)
