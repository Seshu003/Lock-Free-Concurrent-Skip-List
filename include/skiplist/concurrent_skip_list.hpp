#pragma once

#include "marked_ptr.hpp"
#include "hazard_pointer.hpp"

#include <atomic>
#include <cstddef>
#include <random>
#include <vector>
#include <optional>
#include <utility>

namespace skiplist {

inline constexpr std::size_t kMaxLevel = 16;
inline constexpr double kProbability = 0.5;

template<typename K, typename V>
class ConcurrentSkipList {
public:
    struct Node {
        K key;
        V value;
        std::size_t level;
        std::atomic<MarkedPtr<Node>> next[1];

        static Node* create(const K& k, const V& v, std::size_t lvl) {
            std::size_t size = sizeof(Node) + (lvl > 0 ? (lvl - 1) * sizeof(std::atomic<MarkedPtr<Node>>) : 0);
            void* mem = ::operator new(size);
            return ::new (mem) Node(k, v, lvl);
        }

        static void destroy(Node* node) {
            if (!node) return;
            node->~Node();
            ::operator delete(static_cast<void*>(node));
        }

    private:
        Node(const K& k, const V& v, std::size_t lvl)
            : key(k), value(v), level(lvl) {
            for (std::size_t i = 0; i < lvl; ++i) {
                next[i].store(MarkedPtr<Node>(nullptr, false), std::memory_order_relaxed);
            }
        }
    };

    ConcurrentSkipList()
        : head_(Node::create(K{}, V{}, kMaxLevel)),
          tail_(Node::create(K{}, V{}, kMaxLevel)) {
        for (std::size_t i = 0; i < kMaxLevel; ++i) {
            head_->next[i].store(MarkedPtr<Node>(tail_, false), std::memory_order_relaxed);
            tail_->next[i].store(MarkedPtr<Node>(nullptr, false), std::memory_order_relaxed);
        }
    }

    ~ConcurrentSkipList() {
        Node* curr = head_->next[0].load(std::memory_order_relaxed).ptr();
        while (curr && curr != tail_) {
            Node* next = curr->next[0].load(std::memory_order_relaxed).ptr();
            Node::destroy(curr);
            curr = next;
        }
        Node::destroy(head_);
        Node::destroy(tail_);
    }

    ConcurrentSkipList(const ConcurrentSkipList&) = delete;
    ConcurrentSkipList& operator=(const ConcurrentSkipList&) = delete;

    bool find(const K& key, V& value_out) {
        Node* preds[kMaxLevel];
        Node* succs[kMaxLevel];
        return find_position(key, preds, succs, true, value_out);
    }

    bool contains(const K& key) {
        V dummy{};
        return find(key, dummy);
    }

    bool insert(const K& key, const V& val) {
        std::size_t top_level = random_level();
        Node* preds[kMaxLevel];
        Node* succs[kMaxLevel];

        while (true) {
            V dummy{};
            if (find_position(key, preds, succs, false, dummy)) {
                return false;
            }

            Node* new_node = Node::create(key, val, top_level);
            for (std::size_t i = 0; i < top_level; ++i) {
                new_node->next[i].store(MarkedPtr<Node>(succs[i], false), std::memory_order_relaxed);
            }

            MarkedPtr<Node> expected(succs[0], false);
            MarkedPtr<Node> desired(new_node, false);
            if (!preds[0]->next[0].compare_exchange_strong(expected, desired,
                                                           std::memory_order_acq_rel,
                                                           std::memory_order_relaxed)) {
                Node::destroy(new_node);
                continue;
            }

            for (std::size_t i = 1; i < top_level; ++i) {
                while (true) {
                    MarkedPtr<Node> pred_next = preds[i]->next[i].load(std::memory_order_relaxed);
                    MarkedPtr<Node> node_next = new_node->next[i].load(std::memory_order_relaxed);
                    
                    new_node->next[i].store(MarkedPtr<Node>(succs[i], false), std::memory_order_relaxed);
                    
                    MarkedPtr<Node> exp(succs[i], false);
                    MarkedPtr<Node> des(new_node, false);
                    if (preds[i]->next[i].compare_exchange_strong(exp, des,
                                                                   std::memory_order_acq_rel,
                                                                   std::memory_order_relaxed)) {
                        break;
                    }
                    find_position(key, preds, succs, false, dummy);
                }
            }
            return true;
        }
    }

    bool remove(const K& key) {
        Node* preds[kMaxLevel];
        Node* succs[kMaxLevel];

        while (true) {
            V dummy{};
            if (!find_position(key, preds, succs, false, dummy)) {
                return false;
            }

            Node* victim = succs[0];
            std::size_t top_level = victim->level;

            for (std::size_t i = top_level; i-- > 1;) {
                MarkedPtr<Node> current = victim->next[i].load(std::memory_order_relaxed);
                while (!current.mark()) {
                    MarkedPtr<Node> marked(current.ptr(), true);
                    if (victim->next[i].compare_exchange_weak(current, marked,
                                                              std::memory_order_acq_rel,
                                                              std::memory_order_relaxed)) {
                        break;
                    }
                }
            }

            MarkedPtr<Node> current = victim->next[0].load(std::memory_order_relaxed);
            while (true) {
                bool marked = current.mark();
                MarkedPtr<Node> marked_ptr(current.ptr(), true);
                if (marked) break;

                if (victim->next[0].compare_exchange_strong(current, marked_ptr,
                                                             std::memory_order_acq_rel,
                                                             std::memory_order_relaxed)) {
                    V dummy2{};
                    find_position(key, preds, succs, false, dummy2);
                    RetireList<Node>::retire(victim);
                    return true;
                }
            }
            return false;
        }
    }

private:
    std::size_t random_level() {
        thread_local std::mt19937 rng(std::random_device{}());
        thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);

        std::size_t lvl = 1;
        while (dist(rng) < kProbability && lvl < kMaxLevel) {
            ++lvl;
        }
        return lvl;
    }

    bool find_position(const K& key, Node** preds, Node** succs, bool fetch_val, V& val_out) {
        bool retry = false;
        HazardPointer hp_pred(0);
        HazardPointer hp_curr(1);

        do {
            retry = false;
            Node* pred = head_;

            for (std::size_t i = kMaxLevel; i-- > 0;) {
                hp_pred.set(pred);
                MarkedPtr<Node> curr_marked = pred->next[i].load(std::memory_order_acquire);
                Node* curr = curr_marked.ptr();

                while (true) {
                    if (!curr || curr == tail_) {
                        succs[i] = tail_;
                        preds[i] = pred;
                        break;
                    }

                    hp_curr.set(curr);
                    MarkedPtr<Node> succ_marked = curr->next[i].load(std::memory_order_acquire);

                    while (succ_marked.mark()) {
                        MarkedPtr<Node> exp(curr, false);
                        MarkedPtr<Node> des(succ_marked.ptr(), false);
                        if (!pred->next[i].compare_exchange_strong(exp, des,
                                                                    std::memory_order_acq_rel,
                                                                    std::memory_order_relaxed)) {
                            retry = true;
                            break;
                        }
                        curr = succ_marked.ptr();
                        if (!curr || curr == tail_) break;
                        hp_curr.set(curr);
                        succ_marked = curr->next[i].load(std::memory_order_acquire);
                    }

                    if (retry) break;

                    if (curr != tail_ && curr->key < key) {
                        pred = curr;
                        hp_pred.set(pred);
                        curr = succ_marked.ptr();
                    } else {
                        preds[i] = pred;
                        succs[i] = curr;
                        break;
                    }
                }
                if (retry) break;
            }

            if (retry) continue;

            if (succs[0] != tail_ && succs[0]->key == key) {
                if (fetch_val) {
                    val_out = succs[0]->value;
                }
                return true;
            }
            return false;

        } while (retry);

        return false;
    }

    Node* head_{nullptr};
    Node* tail_{nullptr};
};

}
