add_library(conductor_ecs 
SHARED
    ComponentRegistry.cpp
    ComponentSet.cpp
    ComponentStorageRegistry.cpp
    EntityManager.cpp
)

target_include_directories(conductor_ecs
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_ecs
PUBLIC
    conductor_core
    conductor_events
)

target_compile_features(conductor_ecs
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_ecs PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_ecs PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()