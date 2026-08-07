add_library(conductor_scheduler
    ScheduledTask.cpp
    Scheduler.cpp
)

target_include_directories(conductor_scheduler
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/engine
)

target_compile_features(conductor_scheduler
PUBLIC
    cxx_std_23
)

if(MSVC)
    target_compile_options(conductor_scheduler PRIVATE 
        /W4
        /permissive-
    )
else()
    target_compile_options(conductor_scheduler PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()