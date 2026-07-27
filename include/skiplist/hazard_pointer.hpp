#pragma once

#include <atomic>
#include <array>
#include <cstddef>
#include <thread>
#include <vector>
#include <algorithm>

namespace skiplist {

inline constexpr std::size_t kMaxThreads = 128;
inline constexpr std::size_t kHazardsPerThread = 4;

struct HazardPointerDomain {
    struct HPRecord {
        std::atomic<std::thread::id> id{};
        std::array<std::atomic<void*>, kHazardsPerThread> hp{};
    };

    std::array<HPRecord, kMaxThreads> records{};

    HPRecord* acquire_record() {
        thread_local HPRecord* local_rec = nullptr;
        if (local_rec) return local_rec;

        std::thread::id self = std::this_thread::get_id();
        for (auto& rec : records) {
            std::thread::id expected{};
            if (rec.id.compare_exchange_strong(expected, self, std::memory_order_acq_rel)) {
                local_rec = &rec;
                return local_rec;
            }
            if (expected == self) {
                local_rec = &rec;
                return local_rec;
            }
        }
        return nullptr;
    }

    void release_record() {
        thread_local HPRecord* local_rec = nullptr;
        if (local_rec) {
            for (auto& slot : local_rec->hp) {
                slot.store(nullptr, std::memory_order_release);
            }
            local_rec->id.store(std::thread::id{}, std::memory_order_release);
            local_rec = nullptr;
        }
    }
};

inline HazardPointerDomain& get_default_domain() {
    static HazardPointerDomain domain;
    return domain;
}

class HazardPointer {
public:
    explicit HazardPointer(std::size_t index = 0) : index_(index) {
        rec_ = get_default_domain().acquire_record();
    }

    ~HazardPointer() {
        clear();
    }

    template<typename T>
    T* protect(const std::atomic<T*>& src) {
        T* ptr = src.load(std::memory_order_relaxed);
        while (true) {
            set(ptr);
            T* current = src.load(std::memory_order_acquire);
            if (ptr == current) break;
            ptr = current;
        }
        return ptr;
    }

    void set(void* ptr) {
        if (rec_ && index_ < kHazardsPerThread) {
            rec_->hp[index_].store(ptr, std::memory_order_release);
        }
    }

    void clear() {
        if (rec_ && index_ < kHazardsPerThread) {
            rec_->hp[index_].store(nullptr, std::memory_order_release);
        }
    }

private:
    HazardPointerDomain::HPRecord* rec_{nullptr};
    std::size_t index_{0};
};

template<typename T>
class RetireList {
public:
    using DeleterFunc = void (*)(T*);

    struct Entry {
        T* ptr{nullptr};
        DeleterFunc deleter{nullptr};
    };

    static void retire(T* ptr, DeleterFunc deleter = [](T* p) { delete p; }) {
        auto& retired = get_retired_list();
        retired.push_back({ptr, deleter});
        if (retired.size() >= 16) {
            reclaim(retired);
        }
    }

    static void reclaim_all() {
        auto& retired = get_retired_list();
        reclaim(retired);
    }

private:
    static std::vector<Entry>& get_retired_list() {
        thread_local std::vector<Entry> list;
        return list;
    }

    static void reclaim(std::vector<Entry>& retired) {
        if (retired.empty()) return;

        std::vector<void*> hazards;
        auto& domain = get_default_domain();
        for (const auto& rec : domain.records) {
            for (const auto& hp : rec.hp) {
                void* p = hp.load(std::memory_order_acquire);
                if (p) hazards.push_back(p);
            }
        }
        std::sort(hazards.begin(), hazards.end());

        auto it = std::remove_if(retired.begin(), retired.end(), [&](const Entry& entry) {
            if (!std::binary_search(hazards.begin(), hazards.end(), static_cast<void*>(entry.ptr))) {
                if (entry.deleter) {
                    entry.deleter(entry.ptr);
                } else {
                    delete entry.ptr;
                }
                return true;
            }
            return false;
        });
        retired.erase(it, retired.end());
    }
};

}
