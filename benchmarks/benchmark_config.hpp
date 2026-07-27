#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <algorithm>

namespace bench {

inline constexpr std::size_t kDefaultOps = 100'000;

class BenchTimer {
public:
    using Clock = std::chrono::steady_clock;

    void start() { start_ = Clock::now(); }
    void stop()  { end_   = Clock::now(); }

    double elapsed_seconds() const {
        return std::chrono::duration<double>(end_ - start_).count();
    }

    double ops_per_sec(std::size_t ops) const {
        return static_cast<double>(ops) / elapsed_seconds();
    }

private:
    Clock::time_point start_{};
    Clock::time_point end_{};
};

}
