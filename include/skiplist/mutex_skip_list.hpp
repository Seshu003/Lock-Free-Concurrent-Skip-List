#pragma once

#include <mutex>
#include <map>
#include <optional>

namespace skiplist {

template<typename K, typename V>
class MutexSkipList {
public:
    MutexSkipList() = default;

    bool insert(const K& key, const V& val) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto [it, inserted] = map_.insert_or_assign(key, val);
        return inserted;
    }

    bool remove(const K& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.erase(key) > 0;
    }

    bool find(const K& key, V& value_out) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            value_out = it->second;
            return true;
        }
        return false;
    }

    bool contains(const K& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.contains(key);
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.size();
    }

private:
    mutable std::mutex mutex_;
    std::map<K, V> map_;
};

}
