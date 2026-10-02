add_library(conductor_runtime
SHARED
    Application.cpp
)

target_include_directories(conductor_runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_runtime
PUBLIC
    conductor_core
    conductor_events
    conductor_plugins
    conductor_scheduler
    conductor_simulation
)

target_link_libraries(conductor_runtime
PUBLIC
    Taskflow::Taskflow
)

target_compile_features(conductor_runtime
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_runtime PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_runtime PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()