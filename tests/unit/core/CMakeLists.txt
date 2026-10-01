add_executable(core_tests
    TestClock.cpp
    TestLogger.cpp
    TestRandom.cpp
    TestSimulationClock.cpp
    TestTimer.cpp
    TestUUID.cpp
)

target_link_libraries(core_tests
PRIVATE
    conductor_lib
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(core_tests
PROPERTIES
    LABELS "conductor core"
)