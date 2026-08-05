add_executable(core_tests
    TestApplication.cpp
    TestClock.cpp
    TestLogger.cpp
    TestRandom.cpp
    TestTimer.cpp
    TestUUID.cpp
)

target_link_libraries(core_tests
PRIVATE
    conductor_core
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(core_tests)