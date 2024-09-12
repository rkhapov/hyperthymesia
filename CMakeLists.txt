cmake_minimum_required(VERSION 3.16)

project(hyperthymesia C)

set(CMAKE_C_STANDARD_REQUIRED c11)

set(HEADERS )

set(SOURCES )

add_executable(hm_tests
    ${HEADERS}
    ${SOURCES}
    src/tests/main.c
)
target_include_directories(hm_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
