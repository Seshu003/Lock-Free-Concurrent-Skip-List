# Lock-Free Concurrent Skip List in C++20

A thread-safe lock-free skip list implementation in C++20.

## Short project description

This project implements a non-blocking skip list where multiple threads can search, insert, and delete key-value pairs concurrently without using mutexes. It uses atomic Compare-And-Swap (CAS) instructions for node pointer updates and hazard pointers to safely free memory when nodes are deleted.

## Why I built it

I built this project to learn how lock-free concurrency primitives work in modern C++. I wanted to see how atomic marked pointers and hazard pointers prevent race conditions and memory bugs under thread contention.

## Main files

* `ConcurrentSkipList<K, V>`: Lock-free skip list supporting concurrent lookup, insertion, and deletion.
* `MarkedPtr<T>`: Packs a mark bit into pointer low bits for logical node deletion.
* `HazardPointer`: Thread-local hazard pointer registry that stops threads from reading freed memory.
* `MutexSkipList<K, V>`: Simple `std::mutex` wrapped `std::map` used as a baseline for performance comparisons.

## Build

Requirements: CMake 3.20+ and a C++20 compiler (GCC 12+ or Clang 15+).

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -j
```

## Run tests

The test suite uses Catch2 for unit and stress testing.

```bash
./build/tests/test_skip_list_basic
./build/tests/test_hazard_pointers
./build/tests/test_concurrent_stress
```

You can also run tests under ThreadSanitizer:

```bash
cmake -DCMAKE_BUILD_TYPE=Tsan -B build-tsan
cmake --build build-tsan -j
./build-tsan/tests/test_concurrent_stress
```

## Run benchmarks

Run the benchmark script to compare lock-free performance against the mutex baseline:

```bash
bash scripts/run_benchmarks.sh
```

Or run the benchmark binary directly:

```bash
./build/benchmarks/bench_skip_list
```

## Project layout

```text
concurrent-skip-list/
├── README.md
├── CMakeLists.txt
├── .clang-format
├── include/
│   └── skiplist/
│       ├── concurrent_skip_list.hpp
│       ├── hazard_pointer.hpp
│       ├── marked_ptr.hpp
│       └── mutex_skip_list.hpp
├── tests/
│   ├── CMakeLists.txt
│   ├── test_skip_list_basic.cpp
│   ├── test_hazard_pointers.cpp
│   └── test_concurrent_stress.cpp
├── benchmarks/
│   ├── CMakeLists.txt
│   ├── bench_skip_list.cpp
│   └── benchmark_config.hpp
└── scripts/
    ├── run_tests.sh
    └── run_benchmarks.sh
```

## Known limitations

* The hazard pointer array uses a fixed cap of 128 thread slots.
* The memory reclamation scan uses a simple linear search over registered hazard slots.
* The random level generator uses a uniform real distribution instead of a faster bit-counting approach.
