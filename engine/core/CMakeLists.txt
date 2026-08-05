add_library(conductor_core
    Application.cpp
    Clock.cpp
    Logger.cpp
    Random.cpp
    Timer.cpp
    UUID.cpp
)

target_include_directories(conductor_core
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(conductor_core
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_core PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()