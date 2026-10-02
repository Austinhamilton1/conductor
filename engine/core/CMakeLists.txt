add_library(conductor_core 
SHARED
    Clock.cpp
    Executor.cpp
    Logger.cpp
    Random.cpp
    SimulationClock.cpp
    Timer.cpp
    UUID.cpp
)

target_include_directories(conductor_core
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_link_libraries(conductor_core
PRIVATE
    Taskflow::Taskflow
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