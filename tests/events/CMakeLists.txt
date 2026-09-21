add_executable(event_tests
    TestEventBus.cpp
    TestEventDispatcher.cpp
    TestEventQueue.cpp
    TestSubscription.cpp
)

target_link_libraries(event_tests
PRIVATE
    conductor_events
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(event_tests
PROPERTIES
    LABELS "events"
)