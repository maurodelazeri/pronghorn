cmake_minimum_required(VERSION 3.14)
project(pronghorn)
set(CMAKE_VERBOSE_MAKEFILE ON)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
else ()
    add_definitions(-DDEBUG=0)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE GRAPH ${CMAKE_CURRENT_SOURCE_DIR}/src/libs/graph/*)
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/graph")

set ( MISC src/streaming.cc src/streaming.h src/libs/match.h)

add_executable(pronghorn main.cc ${GRAPH} ${MISC})

target_link_libraries(pronghorn
        pthread
        OpenSSL::SSL
        OpenSSL::Crypto)