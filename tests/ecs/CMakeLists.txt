add_executable(ecs_tests
    TestComponentRegistry.cpp
    TestComponentRegistryFreeze.cpp
    TestComponentSet.cpp
    TestEntityManager.cpp
)

target_link_libraries(ecs_tests
PRIVATE
    conductor_ecs
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(ecs_tests)