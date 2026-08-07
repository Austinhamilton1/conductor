#include <catch2/catch_test_macros.hpp>

#include "events/Event.h"
#include "events/EventType.h"
#include "events/EventDispatcher.h"
#include "TestEvent.h"

TEST_CASE("Dispatcher invokes listener") {
    conductor::EventDispatcher dispatcher;

    int value = 0;

    auto sub = dispatcher.subscribe<TestEvent>(
        [&](const TestEvent& e) {
            value = e.value;
        }
    );

    TestEvent payload{10};

    conductor::Event event;

    event.type = conductor::event_type<TestEvent>();
    event.payload = &payload;

    dispatcher.dispatch(event);

    REQUIRE(value == 10);
}

TEST_CASE("Multiple listeners receive event") {
    conductor::EventDispatcher dispatcher;

    int calls = 0;

    auto a = dispatcher.subscribe<TestEvent>(
        [&](const TestEvent&) {
            calls++;
        }
    );

    auto b = dispatcher.subscribe<TestEvent>(
        [&](const TestEvent&) {
            calls++;
        }
    );

    TestEvent payload{};

    conductor::Event event {
        conductor::event_type<TestEvent>(),
        &payload,
        nullptr
    };

    dispatcher.dispatch(event);

    REQUIRE(calls == 2);
}

TEST_CASE("Dispatcher ignores different event types") {
    conductor::EventDispatcher dispatcher;

    bool called = false;

    auto sub = dispatcher.subscribe<TestEvent>(
        [&](const TestEvent&) {
            called = true;
        }
    );

    AnotherEvent payload{};

    conductor::Event event(
        conductor::event_type<AnotherEvent>(),
        &payload,
        nullptr
    );

    dispatcher.dispatch(event);

    REQUIRE_FALSE(called);
}