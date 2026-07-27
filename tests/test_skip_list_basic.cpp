#include <catch2/catch_test_macros.hpp>
#include <skiplist/concurrent_skip_list.hpp>
#include <skiplist/mutex_skip_list.hpp>

using namespace skiplist;

TEST_CASE("ConcurrentSkipList basic insert and find", "[skiplist]") {
    ConcurrentSkipList<int, int> list;

    int val = 0;
    CHECK(!list.find(10, val));
    CHECK(!list.contains(10));

    CHECK(list.insert(10, 100));
    CHECK(list.find(10, val));
    CHECK(val == 100);

    CHECK(!list.insert(10, 200));

    CHECK(list.insert(20, 200));
    CHECK(list.insert(5, 50));

    CHECK(list.find(5, val));
    CHECK(val == 50);
    CHECK(list.find(20, val));
    CHECK(val == 200);
}

TEST_CASE("ConcurrentSkipList basic removal", "[skiplist]") {
    ConcurrentSkipList<int, std::string> list;

    list.insert(1, "one");
    list.insert(2, "two");
    list.insert(3, "three");

    std::string val;
    CHECK(list.find(2, val));
    CHECK(val == "two");

    CHECK(list.remove(2));
    CHECK(!list.find(2, val));
    CHECK(!list.remove(2));

    CHECK(list.find(1, val));
    CHECK(val == "one");
    CHECK(list.find(3, val));
    CHECK(val == "three");
}

TEST_CASE("MutexSkipList basic verification", "[skiplist][mutex]") {
    MutexSkipList<int, int> list;

    int val = 0;
    CHECK(list.insert(1, 10));
    CHECK(list.find(1, val));
    CHECK(val == 10);
    CHECK(list.remove(1));
    CHECK(!list.find(1, val));
}
