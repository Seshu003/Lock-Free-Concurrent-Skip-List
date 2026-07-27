#include <catch2/catch_test_macros.hpp>
#include <skiplist/concurrent_skip_list.hpp>
#include <atomic>
#include <thread>
#include <vector>

using namespace skiplist;

TEST_CASE("ConcurrentSkipList multi-threaded insert and find", "[skiplist][stress]") {
    ConcurrentSkipList<int, int> list;
    constexpr int kThreads = 8;
    constexpr int kItemsPerThread = 2000;

    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            while (!start.load(std::memory_order_acquire)) {}
            int base = t * kItemsPerThread;
            for (int i = 0; i < kItemsPerThread; ++i) {
                list.insert(base + i, (base + i) * 10);
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& th : threads) th.join();

    for (int t = 0; t < kThreads; ++t) {
        int base = t * kItemsPerThread;
        for (int i = 0; i < kItemsPerThread; ++i) {
            int val = 0;
            REQUIRE(list.find(base + i, val));
            CHECK(val == (base + i) * 10);
        }
    }
}

TEST_CASE("ConcurrentSkipList multi-threaded insert and remove mixed", "[skiplist][stress]") {
    ConcurrentSkipList<int, int> list;
    constexpr int kProducers = 4;
    constexpr int kConsumers = 4;
    constexpr int kOps = 5000;

    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    for (int p = 0; p < kProducers; ++p) {
        threads.emplace_back([&, p] {
            while (!start.load(std::memory_order_acquire)) {}
            for (int i = 0; i < kOps; ++i) {
                list.insert(i, i);
            }
        });
    }

    for (int c = 0; c < kConsumers; ++c) {
        threads.emplace_back([&] {
            while (!start.load(std::memory_order_acquire)) {}
            for (int i = 0; i < kOps; ++i) {
                list.remove(i);
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& th : threads) th.join();
}
