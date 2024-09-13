cmake_minimum_required(VERSION 3.16)

project(hyperthymesia C)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "hyperthymesia is сurrently available only under linux systems")
  return()
endif ()

if (NOT DEFINED HT_BT_DEPTH)
    set(HT_BT_DEPTH 3)
endif()
add_compile_definitions(HT_MAX_BT_DEPTH=${HT_BT_DEPTH})

set(CMAKE_C_STANDARD_REQUIRED gnu11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wstrict-aliasing -pedantic")

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
    )
    target_include_directories(ht_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
