cmake_minimum_required(VERSION 3.16)

project(hyperthymesia C)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "hyperthymesia is сurrently available only under linux systems")
    return()
endif ()

if (NOT DEFINED HT_BT_DEPTH)
    set(HT_BT_DEPTH 5)
endif()
add_compile_definitions(HT_MAX_BT_DEPTH=${HT_BT_DEPTH})

set(CLANGCOMPILERSTANDART gnu11)
set(CMAKE_C_STANDARD_REQUIRED gnu11)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(CMAKE_BUILD_TYPE "Debug")
endif()
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -pedantic -Wall -Wextra -Wstrict-aliasing -g -O2 -pthread -D_GNU_SOURCE")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -pedantic -Wall -Wextra -Wstrict-aliasing -g -O0 -pthread -D_GNU_SOURCE")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "ASAN")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -g -fsanitize=address,undefined -fno-sanitize-recover=all -pedantic -Wall -Wextra -Wstrict-aliasing -g -O0 -pthread -D_GNU_SOURCE")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CC_FLAGS}")

set(HEADERS
    include/ht_malloc.h
    include/ht_defer.h
    include/ht_bt.h
    include/ht_table.h
    include/ht_alloc_header.h
    include/ht_alloc_stat.h
)

set(SOURCES
    src/ht_malloc.c
    src/ht_bt.c
    src/ht_table.c
)

add_library(hyperthymesia ${HEADERS} ${SOURCES})
target_include_directories(hyperthymesia PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(HT_BUILD_TESTS "Build hyperthymesia tests" OFF)
if (HT_BUILD_TESTS)
    add_executable(ht_tests
        ${HEADERS}
        ${SOURCES}
        src/tests/main.c
        src/tests/test_ht_malloc_ht_free.c
        src/tests/benchmark.c
        src/tests/test_ht_bt_collect.c
        src/tests/test_ht_realloc.c
    )
    target_include_directories(ht_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
