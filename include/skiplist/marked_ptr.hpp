#pragma once

#include <cstdint>
#include <type_traits>

namespace skiplist {

template<typename T>
class MarkedPtr {
public:
    constexpr MarkedPtr() noexcept : ptr_(0) {}
    constexpr MarkedPtr(T* ptr, bool mark = false) noexcept
        : ptr_(reinterpret_cast<uintptr_t>(ptr) | (mark ? 1ULL : 0ULL)) {}

    T* ptr() const noexcept {
        return reinterpret_cast<T*>(ptr_ & ~1ULL);
    }

    bool mark() const noexcept {
        return (ptr_ & 1ULL) != 0;
    }

    uintptr_t raw() const noexcept {
        return ptr_;
    }

    bool operator==(const MarkedPtr& other) const noexcept {
        return ptr_ == other.ptr_;
    }

    bool operator!=(const MarkedPtr& other) const noexcept {
        return ptr_ != other.ptr_;
    }

private:
    uintptr_t ptr_{0};
};

}
