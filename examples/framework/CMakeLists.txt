add_executable(simulation
    Simulation.cpp
)

target_link_libraries(simulation
PRIVATE
    conductor_commands
    conductor_core
    conductor_ecs
    conductor_events
    conductor_plugins
    conductor_runtime
    conductor_scheduler
    conductor_simulation
)