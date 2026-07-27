# Lock-Free Concurrent Skip List in C++20

Implementation of a thread-safe, non-blocking lock-free skip list using atomic operations, marked pointers, and hazard pointers for safe memory reclamation under concurrent key insertion, lookup, and deletion.

## Features

- **Lock-Free Skip List (`ConcurrentSkipList<K, V>`)**: Multi-level lock-free skip list implementing Herlihy & Shavit style CAS-based pointer updates without mutexes or spinlocks.
- **Atomic Marked Pointers (`MarkedPtr<T>`)**: Utilizes low-order bit packing on 64-bit pointers for two-phase logical deletion and physical unlinking.
- **Lock-Free Memory Reclamation (`HazardPointer`)**: Hazard pointer domain registry preventing ABA problems, use-after-free, and premature node deletion during concurrent traversals.
- **Mutex Baseline (`MutexSkipList<K, V>`)**: Thread-safe mutex-protected std::map baseline for benchmark evaluation.

## Building and Testing

Requirements: CMake 3.20+ and a C++20 compliant compiler (GCC 12+ or Clang 15+).

```bash
# Build Release mode
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -j

# Run test binaries
./build/tests/test_skip_list_basic
./build/tests/test_hazard_pointers
./build/tests/test_concurrent_stress

# Run ThreadSanitizer build for data race verification
cmake -DCMAKE_BUILD_TYPE=Tsan -B build-tsan
cmake --build build-tsan -j
./build-tsan/tests/test_concurrent_stress
```

## Running Benchmarks

```bash
bash scripts/run_benchmarks.sh
```

## Project Structure

```text
include/skiplist/
  marked_ptr.hpp          bit-packed atomic pointer marking
  hazard_pointer.hpp      lock-free hazard pointer registry and retire list
  concurrent_skip_list.hpp lock-free skip list implementation
  mutex_skip_list.hpp     std::mutex baseline wrapper
tests/
  test_skip_list_basic.cpp basic functional correctness
  test_hazard_pointers.cpp hazard pointer memory reclamation tests
  test_concurrent_stress.cpp 8-thread concurrent stress testing
benchmarks/
  benchmark_config.hpp    timing harness
  bench_skip_list.cpp     throughput comparison against mutex baseline
```
