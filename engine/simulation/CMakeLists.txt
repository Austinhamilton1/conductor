add_library(conductor_simulation
    
)

target_include_directories(conductor_simulation
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_compile_features(conductor_simulation
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_simulation PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_simulation PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()