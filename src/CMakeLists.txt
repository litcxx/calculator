set(SOURCES
    application.cpp
    logger.cpp
    parser.cpp
)

add_library(calc_core STATIC
    ${SOURCES}
)

target_include_directories(calc_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(calc_core
    PRIVATE third_party
    PUBLIC calc_warnings
)
