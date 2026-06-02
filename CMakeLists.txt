cmake_minimum_required(VERSION 3.15...4.3)

project(calc VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# --- COMMON COMPILER SETTINGS ---
add_library(calc_warnings INTERFACE)

set(DEBUG_FLAGS
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wold-style-cast -Werror -g
    -fsanitize=address,undefined
)

target_compile_options(calc_warnings INTERFACE
    "$<$<CONFIG:Debug>:${DEBUG_FLAGS}>"
    $<$<CONFIG:Release>:-O2>
)

target_link_options(calc_warnings INTERFACE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# --- SUBDIRECTORIES ---
add_subdirectory(third_party)
add_subdirectory(src)

# --- MAIN EXECUTABLE ---
add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    calc_core
    calc_warnings
)

# --- TESTS ---
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test/unit)
endif()

# --- CLANG-FORMAT ---
if (USE_CLANG_FORMAT)
    find_program(CLANG_FORMAT NAMES clang-format-21)
    if (CLANG_FORMAT)
        message(STATUS "Found clang-format-21: ${CLANG_FORMAT}")

        file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/*.cpp
        )

        add_custom_target(clang-format-fix
            COMMAND ${CLANG_FORMAT} -style=file -i ${CLANG_FORMAT_SRC}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Formatting ${PROJECT_NAME} with ${CLANG_FORMAT} ..."
        )

        add_custom_target(clang-format
            COMMAND run-clang-format.py --clang-format-executable=${CLANG_FORMAT} --style=file ${CLANG_FORMAT_SRC}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking format ${PROJECT_NAME} with ${CLANG_FORMAT} ..."
        )
    endif()
endif()

# --- CLANG-TIDY ---
if (USE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy-21)
    if(CLANG_TIDY)
        message(STATUS "Found clang-tidy-21: ${CLANG_TIDY}")

        set(CLANG_TIDY_COMMAND ${CLANG_TIDY} "-config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy")
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
        set_target_properties(calc_core PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    endif()
endif()

# --- SETUP INSTALL ---
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
