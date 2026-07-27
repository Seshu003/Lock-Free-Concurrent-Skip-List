#include <catch2/catch_test_macros.hpp>
#include <skiplist/hazard_pointer.hpp>
#include <thread>
#include <vector>

using namespace skiplist;

struct DummyNode {
    int value{0};
};

TEST_CASE("HazardPointer acquire and release", "[hazard_pointer]") {
    HazardPointer hp(0);
    DummyNode node{42};
    hp.set(&node);

    hp.clear();
}

TEST_CASE("RetireList safe reclamation", "[hazard_pointer]") {
    DummyNode* node1 = new DummyNode{1};
    DummyNode* node2 = new DummyNode{2};

    RetireList<DummyNode>::retire(node1);
    RetireList<DummyNode>::retire(node2);
    RetireList<DummyNode>::reclaim_all();
}
