cmake_minimum_required(VERSION 3.16)

project(hyperthymesia C)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "hyperthymesia is сurrently available only under linux systems")
  return()
endif ()

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

add_executable(ht_tests
    ${HEADERS}
    ${SOURCES}
    src/tests/main.c
)
target_include_directories(ht_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
