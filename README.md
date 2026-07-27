# Lock-Free Concurrent Skip List in C++20

I implemented a lock-free concurrent skip list in C++20. It allows multiple threads to search, insert, and delete key-value pairs concurrently without using mutexes.

## What this project does

This project provides a concurrent skip list data structure where threads do not block each other using locks. It uses atomic CAS instructions to update forward pointers and hazard pointers to safely clean up deleted nodes.

## Why I built it

I built this project to understand how non-blocking concurrent data structures work in practice. I wanted to see how atomic marked pointers and hazard pointers interact under actual thread contention.

## Main parts

* `ConcurrentSkipList<K, V>`: A lock-free skip list that supports concurrent search, insertion, and removal using Compare-And-Swap.
* `MarkedPtr<T>`: Packs a boolean mark bit into pointer low bits for two-phase node deletion.
* `HazardPointer`: A thread-local hazard pointer registry that prevents reading threads from accessing freed memory.
* `MutexSkipList<K, V>`: A simple mutex-wrapped map used as a baseline to measure speedups.

## Build and run

You need CMake 3.20+ and a C++20 compiler.

```bash
# Build release binaries
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -j

# Build ThreadSanitizer version
cmake -DCMAKE_BUILD_TYPE=Tsan -B build-tsan
cmake --build build-tsan -j
```

## Tests

The test suite uses Catch2 to test functionality and concurrency.

```bash
./build/tests/test_skip_list_basic
./build/tests/test_hazard_pointers
./build/tests/test_concurrent_stress
```

You can also run tests under ThreadSanitizer:

```bash
./build-tsan/tests/test_concurrent_stress
```

## Benchmarks

Run the benchmark script to compare performance against the mutex version:

```bash
bash scripts/run_benchmarks.sh
```

Or run the binary directly:

```bash
./build/benchmarks/bench_skip_list
```

Sample output files are saved in the `results/` folder.

## Project layout

```text
include/skiplist/
  marked_ptr.hpp          bit-packed atomic pointers
  hazard_pointer.hpp      hazard pointer memory reclamation
  concurrent_skip_list.hpp lock-free skip list
  mutex_skip_list.hpp     mutex baseline
tests/
  test_skip_list_basic.cpp basic functional tests
  test_hazard_pointers.cpp hazard pointer unit tests
  test_concurrent_stress.cpp multi-thread stress tests
benchmarks/
  benchmark_config.hpp    timing helpers
  bench_skip_list.cpp     throughput comparison
docs/
  dev-notes.md            development notes and reflections
results/
  benchmark_sample.txt    sample benchmark run output
  test_sample.txt         sample test and sanitizer output
```

## What I found difficult

The hardest part was fixing a race condition during node removal. In early tests, readers occasionally crashed when traversing a node that was being unlinked by another thread. I realized I was marking the level 0 pointer before higher level pointers. Reversing the marking order so level 0 is marked last solved the issue.

For design trade-offs, I chose hazard pointers over epoch-based reclamation. Hazard pointers give a tighter bound on memory usage because retired nodes get reclaimed as soon as active readers finish. The downside is that checking hazard pointer arrays on every retire step adds extra overhead during deletion.

## Known limitations

The hazard pointer array uses a fixed array of 128 thread slots. If a program spawns more than 128 threads, record allocation fails. Also, the current implementation has not been tested on non-x86 hardware architectures with relaxed memory models.
