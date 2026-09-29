add_executable(conductor
    main.cpp
    parser.cpp
)

target_link_libraries(conductor
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

target_link_libraries(conductor
PRIVATE
    yaml-cpp
)