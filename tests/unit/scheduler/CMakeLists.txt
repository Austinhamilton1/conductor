add_executable(scheduler_tests
    TestScheduler.cpp
    TestSystems.cpp
)

target_link_libraries(scheduler_tests
PRIVATE
    conductor_lib
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(scheduler_tests
PROPERTIES
    LABELS "conductor scheduler"
)