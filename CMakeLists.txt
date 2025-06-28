cmake_minimum_required(VERSION 3.10)

project(rotLangVM C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)


add_executable(rotlangvm
    main.c
    chunk.c
    debug.c
    memory.c
    value.c
    vm.c
    compiler.c
    scanner.c
)
