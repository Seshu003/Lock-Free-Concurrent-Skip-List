cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)

FetchContent_MakeAvailable(Catch2)

function(add_skiplist_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE skiplist_core Catch2::Catch2WithMain)
endfunction()

add_skiplist_test(test_skip_list_basic   test_skip_list_basic.cpp)
add_skiplist_test(test_hazard_pointers   test_hazard_pointers.cpp)
add_skiplist_test(test_concurrent_stress test_concurrent_stress.cpp)
