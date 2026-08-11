add_library(conductor_commands
    Command.cpp
    CommandQueue.cpp
)

target_include_directories(conductor_commands
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_commands
PUBLIC
    conductor_ecs
)

target_compile_features(conductor_commands
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_commands PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_commands PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()