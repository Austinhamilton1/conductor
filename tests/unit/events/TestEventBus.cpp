#include <catch2/catch_test_macros.hpp>

#include "events/EventBus.h"
#include "TestEvent.h"

TEST_CASE("Publish dispatches event", "[events]") {
    conductor::EventBus bus;

    int value = 0;

    auto sub = bus.subscribe<TestEvent>(
        [&](const TestEvent& e) {
            value = e.value;
        }
    );

    bus.publish(TestEvent{42});

    bus.process();

    REQUIRE(value == 42);
}

TEST_CASE("Multiple events processed", "[events]") {
    conductor::EventBus bus;

    int sum = 0;

    auto sub = bus.subscribe<TestEvent>(
        [&](const TestEvent& e) {
            sum += e.value;
        }
    );

    bus.publish(TestEvent{1});
    bus.publish(TestEvent{2});
    bus.publish(TestEvent{3});

    bus.process();

    REQUIRE(sum == 6);
}

TEST_CASE("Different event types dispatch independently", "[events]") {
    conductor::EventBus bus;

    bool a = false;
    bool b = false;

    auto sa = bus.subscribe<TestEvent>(
        [&](const TestEvent&) {
            a = true;
        }
    );

    auto sb = bus.subscribe<AnotherEvent>(
        [&](const AnotherEvent&) {
            b = true;
        }
    );

    bus.publish(TestEvent{});

    bus.process();

    REQUIRE(a);
    REQUIRE_FALSE(b);
}

TEST_CASE("Events published during dispatch are processed next cycle", "[events]") {
    conductor::EventBus bus;

    int calls = 0;

    auto sub = bus.subscribe<TestEvent>(
        [&](const TestEvent&) {
            calls++;

            if (calls == 1)
                bus.publish(TestEvent{});
        }
    );

    bus.publish(TestEvent{});

    bus.process();

    REQUIRE(calls == 1);

    bus.process();

    REQUIRE(calls == 2);
}