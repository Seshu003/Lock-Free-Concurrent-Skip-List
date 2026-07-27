#include "benchmark_config.hpp"
#include <skiplist/concurrent_skip_list.hpp>
#include <skiplist/mutex_skip_list.hpp>

#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

using namespace skiplist;

template<typename List>
static double run_benchmark(int threads_count, std::size_t ops_per_thread) {
    List list;
    std::size_t total_ops = threads_count * ops_per_thread;

    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < threads_count; ++t) {
        threads.emplace_back([&, t] {
            while (!start.load(std::memory_order_acquire)) {}
            int base = t * ops_per_thread;
            for (std::size_t i = 0; i < ops_per_thread; ++i) {
                list.insert(base + static_cast<int>(i), static_cast<int>(i));
            }
        });
    }

    bench::BenchTimer timer;
    timer.start();
    start.store(true, std::memory_order_release);

    for (auto& th : threads) th.join();
    timer.stop();

    return timer.ops_per_sec(total_ops);
}

int main() {
    constexpr std::size_t ops_per_thread = 50'000;
    std::printf("Concurrent SkipList vs Mutex SkipList Benchmark\n");
    std::printf("Ops per thread: %zu\n\n", ops_per_thread);

    for (int t : {1, 2, 4, 8}) {
        double lf_tput  = run_benchmark<ConcurrentSkipList<int, int>>(t, ops_per_thread);
        double mtx_tput = run_benchmark<MutexSkipList<int, int>>(t, ops_per_thread);

        std::printf("Threads: %d\n", t);
        std::printf("  Lock-Free SkipList : %10.2f ops/sec\n", lf_tput);
        std::printf("  Mutex Baseline     : %10.2f ops/sec\n", mtx_tput);
        std::printf("  Speedup            : %10.2fx\n\n", lf_tput / mtx_tput);
    }

    return 0;
}
