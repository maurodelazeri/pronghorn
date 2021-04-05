cmake_minimum_required(VERSION 3.14)
project(pronghorn)
set(CMAKE_VERBOSE_MAKEFILE ON)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
else ()
    add_definitions(-DDEBUG=0)
endif()

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

set(LIB_PREFIX "/usr/local/lib")

find_package(Threads REQUIRED)

find_package(OpenSSL REQUIRED)

find_package(Libwebsockets)
set(WEBSOCKET_LIB "${LIB_PREFIX}/libwebsockets.a")

file(GLOB_RECURSE WEBSOCKET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/libs/ws_client/*)
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/ws_client")

add_executable(pronghorn main.cc ${WEBSOCKET_SRC})

target_link_libraries(pronghorn
        ${WEBSOCKET_LIB}
        ${OPENSSL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT})