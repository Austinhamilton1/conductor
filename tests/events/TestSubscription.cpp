#include <catch2/catch_test_macros.hpp>
#include <utility>

#include "events/Event.h"
#include "events/EventType.h"
#include "events/EventDispatcher.h"
#include "events/Subscription.h"
#include "TestEvent.h"

TEST_CASE("Subscription automatically unsubscribes") {
    conductor::EventDispatcher dispatcher;

    int calls = 0;

    {
        auto sub = dispatcher.subscribe<TestEvent>(
            [&](const TestEvent&) {
                calls++;
            }
        );
    }

    TestEvent payload{};

    conductor::Event event {
        conductor::event_type<TestEvent>(),
        &payload,
        nullptr
    };

    dispatcher.dispatch(event);

    REQUIRE(calls == 0);
}

TEST_CASE("Subscription move constructor transfers ownership") {
    conductor::EventDispatcher dispatcher;

    auto sub = dispatcher.subscribe<TestEvent>(
        [](const TestEvent&){}
    );

    auto moved = std::move(sub);

    SUCCEED();
}

TEST_CASE("Subscription move assignment") {
    conductor::EventDispatcher dispatcher;

    auto a = dispatcher.subscribe<TestEvent>(
        [](const TestEvent&){}
    );

    conductor::Subscription b;

    b = std::move(a);

    SUCCEED();
}