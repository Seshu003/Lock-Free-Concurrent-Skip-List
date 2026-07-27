cmake_minimum_required(VERSION 3.20)
project(concurrent_skip_list VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Asan")
    add_compile_options(-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Tsan")
    add_compile_options(-O1 -g -fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

set(SKIPLIST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_library(skiplist_core INTERFACE)
target_include_directories(skiplist_core INTERFACE ${SKIPLIST_INCLUDE_DIR})
target_compile_features(skiplist_core INTERFACE cxx_std_20)

add_subdirectory(tests)
add_subdirectory(benchmarks)
