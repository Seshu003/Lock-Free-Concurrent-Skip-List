cmake_minimum_required(VERSION 3.20)

find_package(Threads REQUIRED)

function(add_skiplist_bench name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE skiplist_core Threads::Threads)
    target_compile_options(${name} PRIVATE -O3)
endfunction()

add_skiplist_bench(bench_skip_list bench_skip_list.cpp)
