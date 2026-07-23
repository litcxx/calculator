cmake_minimum_required(VERSION 3.15...4.3)

project(calc VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# --- COMMON COMPILER SETTINGS ---
add_library(calc_warnings INTERFACE)

option(USE_SANITIZERS "Enable Address and Undefined sanitizers" ON)
option(USE_VALGRIND "Enable Valgrind targets (automatically disables sanitizers)" OFF)

if(USE_VALGRIND AND USE_SANITIZERS)
    message(STATUS "Valgrind is enabled. Disabling conflicting sanitizers.")
    set(USE_SANITIZERS OFF)
endif()

option(USE_TSAN "Enable ThreadSanitizer (mutually exclusive with ASan)" OFF)

if(USE_TSAN)
    set(USE_SANITIZERS OFF)
endif()

set(DEBUG_FLAGS
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wold-style-cast -Werror -g
    -fno-omit-frame-pointer
)

if(USE_SANITIZERS)
    list(APPEND DEBUG_FLAGS -fsanitize=address,undefined)
endif()

if(USE_TSAN)
    # -Wno-tsan: silence the benign "atomic_thread_fence not supported" note that
    # library headers trigger under TSan; -Werror would otherwise reject it.
    list(APPEND DEBUG_FLAGS -fsanitize=thread -Wno-tsan)
endif()

target_compile_options(calc_warnings INTERFACE
    "$<$<CONFIG:Debug>:${DEBUG_FLAGS}>"
    $<$<CONFIG:Release>:-O3 -fno-omit-frame-pointer -g>
)

if(USE_SANITIZERS)
    target_link_options(calc_warnings INTERFACE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
endif()

if(USE_TSAN)
    target_link_options(calc_warnings INTERFACE
        $<$<CONFIG:Debug>:-fsanitize=thread>
    )
endif()

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
    add_subdirectory(test)
endif()

# --- CLANG-FORMAT ---
if (USE_CLANG_FORMAT)
    find_program(CLANG_FORMAT NAMES clang-format-21)
    if (CLANG_FORMAT)
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
    else()
        message(WARNING "clang-format-21 is not found: ${CLANG_FORMAT}")
    endif()
endif()

# --- CLANG-TIDY ---
if (USE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy-21)

    if(CLANG_TIDY)
        file(GLOB_RECURSE CLANG_TIDY_SRC CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        )

        add_custom_target(clang-tidy
            COMMAND ${CLANG_TIDY}
                    -p ${CMAKE_BINARY_DIR}
                    -config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy
                    ${CLANG_TIDY_SRC}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy ..."
        )
    else()
        message(WARNING "clang-tidy-21 is not found: ${CLANG_TIDY}")
    endif()
endif()

# --- COVERAGE CONFIGURATION ---
add_library(coverage_flags INTERFACE)

if (ENABLE_COVERAGE)

    target_compile_options(coverage_flags INTERFACE
        --coverage
        -O0
        -g
        -fprofile-arcs
        -ftest-coverage
        -fno-elide-constructors
    )

    target_link_options(coverage_flags INTERFACE
        --coverage
    )

    find_program(GCOVR_BIN gcovr)
    find_program(GCOV_BIN NAMES gcov-14 gcov)

    if (GCOVR_BIN AND GCOV_BIN)

        set(GCOVR_FILTERS
            --filter "${CMAKE_SOURCE_DIR}/src/"
            --filter "${CMAKE_SOURCE_DIR}/include/"
            --exclude-throw-branches
        )

        add_custom_target(coverage-report
            COMMAND ${GCOVR_BIN}
                --root ${CMAKE_SOURCE_DIR}
                --object-directory ${CMAKE_BINARY_DIR}
                ${GCOVR_FILTERS}
                --gcov-executable ${GCOV_BIN}
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_target(coverage-html
            COMMAND ${GCOVR_BIN}
                --root ${CMAKE_SOURCE_DIR}
                --object-directory ${CMAKE_BINARY_DIR}
                ${GCOVR_FILTERS}
                --gcov-executable ${GCOV_BIN}
                --html-details coverage.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

    elseif(NOT GCOVR_BIN)
        message(WARNING "gcovr is not found")

    elseif(NOT GCOV_BIN)
        message(WARNING "gcov is not found")
    endif()

endif()

# --- SETUP INSTALL ---
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES packaging/calc.service
    DESTINATION /lib/systemd/system
)
install(FILES packaging/calc.env
    DESTINATION /etc/calc
)

# --- CPACK (deb package) ---
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "calc")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "calc calculation service")
set(CPACK_PACKAGE_CONTACT "maintainer@example.com")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")           # binary -> /usr/bin

set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# Runtime deps auto-detected from the linked shared libraries (libpq, libstdc++, ...).
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# Maintainer scripts (create service user, daemon-reload, stop on removal) and
# the conffiles list so dpkg preserves edits to /etc/calc/calc.env on upgrade.
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/deb/postinst"
    "${CMAKE_SOURCE_DIR}/packaging/deb/prerm"
    "${CMAKE_SOURCE_DIR}/packaging/deb/postrm"
    "${CMAKE_SOURCE_DIR}/packaging/deb/conffiles")

include(CPack)
