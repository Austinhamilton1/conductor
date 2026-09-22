add_executable(integration_tests
    TestEngine.cpp
)

target_link_libraries(integration_tests
PRIVATE
    conductor_commands
    conductor_core
    conductor_ecs
    conductor_events
    conductor_plugins
    conductor_runtime
    conductor_simulation
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(integration_tests
PROPERTIES
    LABELS "integration"
)