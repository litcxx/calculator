# calculator

Simple C++ cli-calculator for integeres

## Features

- add
- subtract
- multiply
- divide
- pow
- factorial

## Requirements

- CMake 3.15+
- Clang 21
- GCC/Clang compiler

## Install
```bash
cd calculator
cmake -B build -DCMAKE_BUILD_TYPE=Release
sudo cmake --build build --target install
```

## Help
```bash
calc --help
```


## Build

```bash
cd calculator
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DUSE_CLANG_FORMAT=ON -DUSE_CLANG_TIDY=ON -DBUILD_TESTING=ON
cmake --build build
```

## Run tests
```bash
ctest --test-dir build
```
