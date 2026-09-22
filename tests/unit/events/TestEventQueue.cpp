#include <catch2/catch_test_macros.hpp>

#include "events/EventQueue.h"
#include "TestEvent.h"

TEST_CASE("EventQueue starts empty", "[events]") {
    conductor::EventQueue queue;

    REQUIRE(queue.empty());
}

TEST_CASE("Push adds event", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{5});

    REQUIRE_FALSE(queue.empty());
}

TEST_CASE("Pop returns correct payload", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{42});

    auto event = queue.pop();

    auto *payload = static_cast<TestEvent *>(event.payload);

    REQUIRE(payload->value == 42);

    event.destroy(event.payload);
}

TEST_CASE("Queue preserves FIFO order", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{1});
    queue.push(TestEvent{2});
    queue.push(TestEvent{3});

    auto e1 = queue.pop();
    auto e2 = queue.pop();
    auto e3 = queue.pop();

    REQUIRE(static_cast<TestEvent *>(e1.payload)->value == 1);
    REQUIRE(static_cast<TestEvent *>(e2.payload)->value == 2);
    REQUIRE(static_cast<TestEvent *>(e3.payload)->value == 3);

    e1.destroy(e1.payload);
    e2.destroy(e2.payload);
    e3.destroy(e3.payload);
}

TEST_CASE("Clear removes all events", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{1});
    queue.push(TestEvent{2});

    queue.clear();

    REQUIRE(queue.empty());
}