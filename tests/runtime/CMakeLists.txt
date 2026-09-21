add_executable(runtime_tests
    TestApplication.cpp
)

target_link_libraries(runtime_tests
PRIVATE
    conductor_runtime
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(runtime_tests
PROPERTIES
    LABELS "runtime"
)