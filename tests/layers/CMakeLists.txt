add_executable(layers_tests
    TestLayers.cpp
)

target_link_libraries(layers_tests
PRIVATE
    conductor_layers
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(layers_tests)