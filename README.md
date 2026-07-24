# calculator

A C++ TCP service that evaluates integer calculations, backed by PostgreSQL and packaged as a systemd unit.

## Features

- add
- subtract
- multiply
- divide
- pow
- factorial

## Requirements

- CMake 3.15+
- GCC 10+ (C++17 compiler)

### Optional tools
- Clang 21+ (clang-tidy, clang-format)

## Install
```bash
cd calculator
cmake -B build -DCMAKE_BUILD_TYPE=Release
sudo cmake --build build --target install
```

## Run

`calc` is a long-running service: with the `DB_*` and `CALC_PORT` (default 5555)
environment variables set, it serves calculation requests over TCP. Send one
JSON request per line and read one response line back:

```bash
$ printf '{"first":2,"second":3,"operation":"add"}\n' | nc localhost 5555
Success : 5
```

Operations: `add`, `sub`, `mul`, `div`, `pow`, `fact`. It is also packaged as a
systemd service (see `packaging/`).


## Build

```bash
cd calculator
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DUSE_CLANG_FORMAT=ON -DUSE_CLANG_TIDY=ON -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
cmake --build build -j8
```

## Set env

Required for the `integration` and `commit` tests, which run against a real PostgreSQL.

```bash
export DB_HOST=HOST
export DB_PORT=PORT
export DB_NAME=DBNAME
export DB_USER=USER
export DB_PASSWORD=PASSWORD
```

## Run tests
```bash
ctest -L unit --test-dir build -j4 --output-on-failure
ctest -L integration --test-dir build -j4 --output-on-failure
ctest -L commit --test-dir build -j4 --output-on-failure
ctest -L network --test-dir build -j4 --output-on-failure
```

## Lint & format

Enabled by `-DUSE_CLANG_TIDY=ON` / `-DUSE_CLANG_FORMAT=ON` (Clang 21 required):

```bash
cmake --build build --target clang-tidy        # static analysis
cmake --build build --target clang-format      # check formatting
cmake --build build --target clang-format-fix  # apply formatting in place
```

## Coverage

Build with `-DENABLE_COVERAGE=ON`, run the tests, then generate a report
(gcov is auto-selected to match the compiler):

```bash
cmake --build build --target coverage-report   # summary in the terminal
cmake --build build --target coverage-html      # writes build/coverage.html
```
