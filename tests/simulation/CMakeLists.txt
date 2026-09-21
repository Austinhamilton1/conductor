add_executable(simulation_tests
    TestWorld.cpp
)

target_link_libraries(simulation_tests
PRIVATE
    conductor_simulation
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(simulation_tests
PROPERTIES
    LABELS "simulation"
)