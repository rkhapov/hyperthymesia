cmake_minimum_required(VERSION 3.16)

project(hyperthymesia C)

set(PROJECT_NAME hyperthymesia)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "hyperthymesia is сurrently available only under linux systems")
    return()
endif ()

set(CLANGCOMPILERSTANDART gnu11)
set(CMAKE_C_STANDARD_REQUIRED gnu11)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -pedantic -Wall -Wextra -Wstrict-aliasing -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-builtin-calloc -fno-builtin-malloc -fno-builtin-realloc -fno-builtin-free -O2 -pthread -D_GNU_SOURCE")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -pedantic -Wall -Wextra -Wstrict-aliasing -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-builtin-calloc -fno-builtin-malloc -fno-builtin-realloc -fno-builtin-free -O0 -pthread -D_GNU_SOURCE")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "ASAN")
    set(CMAKE_C_FLAGS "-std=${CLANGCOMPILERSTANDART} -g -fsanitize=address,undefined -fno-sanitize-recover=all -pedantic -Wall -Wextra -Wstrict-aliasing -g -fno-omit-frame-pointer -O0 -pthread -D_GNU_SOURCE")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CC_FLAGS}")

if (NOT DEFINED HT_BT_DEPTH)
    set(HT_BT_DEPTH 15)
endif()
add_compile_definitions(HT_MAX_BT_DEPTH=${HT_BT_DEPTH})
execute_process(
    COMMAND git rev-list --count HEAD
    OUTPUT_VARIABLE VERSION_NUMBER
    RESULT_VARIABLE VERSION_NUMBER_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT "${VERSION_NUMBER_RESULT}" STREQUAL "0")
    message(FATAL_ERROR "can't get version number")
endif()
execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE VERSION_GIT
    RESULT_VARIABLE VERSION_GIT_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT "${VERSION_GIT_RESULT}" STREQUAL "0")
    message(FATAL_ERROR "can't get git patch version")
endif()
set(HT_VERSION "${VERSION_NUMBER}-${VERSION_GIT}-d${HT_BT_DEPTH}")
add_definitions(-DHT_VERSION="${HT_VERSION}")
message("Hyperthymesia version=${HT_VERSION}")

set(HEADERS
    include/ht_malloc.h
    include/ht_bt.h
    include/ht_table.h
    include/ht_alloc_header.h
    include/ht_alloc_stat.h
    include/ht_real_funcs.h
    include/ht_log.h
    include/ht_server.h
    include/ht_hints.h
)

set(SOURCES
    src/ht_malloc.c
    src/ht_bt.c
    src/ht_table.c
    src/ht_real_funcs.c
    src/ht_log.c
    src/ht_server.c
    src/ht_hints.c
)

add_library(hyperthymesia SHARED ${HEADERS} ${SOURCES})
target_include_directories(hyperthymesia PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(hyperthymesia dl)

add_executable(benchmark tests/benchmark.c)
target_link_libraries(benchmark dl)

add_executable(benchmark_linked tests/benchmark.c)
target_link_libraries(benchmark_linked hyperthymesia)
target_link_libraries(benchmark_linked dl)

add_executable(test_leak tests/test_leak.c)
target_link_libraries(test_leak dl)

add_executable(test_fork tests/test_fork.c)
target_link_libraries(test_fork dl)

configure_file(tests/run.py ${CMAKE_CURRENT_BINARY_DIR}/run.py COPYONLY)
configure_file(tools/sorter.py ${CMAKE_CURRENT_BINARY_DIR}/sorter.py COPYONLY)

install(TARGETS hyperthymesia DESTINATION /opt/hyperthymesia/lib)

include(DebianPackage)
