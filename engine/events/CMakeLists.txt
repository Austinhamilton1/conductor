add_library(conductor_events
    EventBus.cpp
    EventDispatcher.cpp
    EventQueue.cpp
    Subscription.cpp
)

target_include_directories(conductor_events
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_events
PUBLIC
    conductor_core
)

target_compile_features(conductor_events
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_events PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_events PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()