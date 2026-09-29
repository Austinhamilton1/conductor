add_library(test_plugin
SHARED
    plugins/TestPlugin.cpp
)

target_link_libraries(test_plugin
PRIVATE
    conductor_runtime
)

add_executable(plugin_tests
    TestPlugin.cpp
)

target_link_libraries(plugin_tests
PRIVATE
    conductor_runtime
    conductor_simulation
    Catch2::Catch2WithMain
)

add_dependencies(plugin_tests test_plugin)

target_compile_definitions(plugin_tests
PRIVATE
    TEST_PLUGIN_PATH="$<TARGET_FILE:test_plugin>"
)

include(Catch)

catch_discover_tests(plugin_tests
PROPERTIES
    LABELS "plugins"
)