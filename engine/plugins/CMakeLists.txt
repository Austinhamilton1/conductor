add_library(conductor_plugins
    PluginManager.cpp
)

target_include_directories(conductor_plugins
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_plugins
PUBLIC
    conductor_core
    conductor_simulation
)

target_compile_features(conductor_plugins
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_plugins PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_plugins PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()