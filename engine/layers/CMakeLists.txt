add_library(conductor_layers
    LayerStack.cpp
)

target_include_directories(conductor_layers
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_layers
PUBLIC
    conductor_core
    conductor_events
    conductor_scheduler
)

target_compile_features(conductor_layers
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_layers PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_layers PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()