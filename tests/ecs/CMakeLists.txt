add_executable(ecs_tests
    TestComponentRegistry.cpp
    TestComponentSet.cpp
    TestComponentStorage.cpp
    TestComponentStorageRegistry.cpp
    TestComponentView.cpp
    TestEntityManager.cpp
    TestQuery.cpp
    TestSystems.cpp
)

target_link_libraries(ecs_tests
PRIVATE
    conductor_ecs
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(ecs_tests)