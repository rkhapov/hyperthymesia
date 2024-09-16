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
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -pedantic -Wall -Wextra -Wstrict-aliasing -g -fno-omit-frame-pointer -O2 -pthread -D_GNU_SOURCE")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -pedantic -Wall -Wextra -Wstrict-aliasing -g -fno-omit-frame-pointer -O0 -pthread -D_GNU_SOURCE")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "ASAN")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -g -fsanitize=address,undefined -fno-sanitize-recover=all -pedantic -Wall -Wextra -Wstrict-aliasing -g -fno-omit-frame-pointer -O0 -pthread -D_GNU_SOURCE")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CC_FLAGS}")

set(HEADERS
    include/ht_malloc.h
    include/ht_bt.h
    include/ht_table.h
    include/ht_alloc_header.h
    include/ht_alloc_stat.h
    include/ht_real_funcs.h
    include/ht_log.h
    include/ht_server.h
)

set(SOURCES
    src/ht_malloc.c
    src/ht_bt.c
    src/ht_table.c
    src/ht_real_funcs.c
    src/ht_log.c
    src/ht_server.c
)

add_library(hyperthymesia SHARED ${HEADERS} ${SOURCES})
target_include_directories(hyperthymesia PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(hyperthymesia unwind)

add_executable(benchmark tests/benchmark.c)

add_executable(benchmark_linked tests/benchmark.c)
target_link_libraries(benchmark_linked hyperthymesia)

add_executable(test_leak tests/test_leak.c)

configure_file(tests/run.py ${CMAKE_CURRENT_BINARY_DIR}/run.py COPYONLY)
