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
)

set(SOURCES
    src/ht_malloc.c
)

add_executable(hm_tests
    ${HEADERS}
    ${SOURCES}
    src/tests/main.c
)
target_include_directories(hm_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
