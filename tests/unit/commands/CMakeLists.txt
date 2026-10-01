add_executable(commands_tests
    TestCommandQueue.cpp
)

target_link_libraries(commands_tests
PRIVATE
    conductor_lib
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(commands_tests
PROPERTIES
    LABELS "conductor commands"
)