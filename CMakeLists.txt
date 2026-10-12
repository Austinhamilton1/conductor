cmake_minimum_required(VERSION 3.25)

project(
    Conductor
    VERSION 0.1.0
    DESCRIPTION "Distributed Coordination Simulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/Catch2)

include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.9.0
)

FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
    taskflow
    GIT_REPOSITORY https://github.com/taskflow/taskflow.git
    GIT_TAG v4.0.0
)

set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Disable taskflow examples" FORCE)
set(TF_BUILD_TESTS OFF CACHE BOOL "Disable taskflow unit tests" FORCE)

FetchContent_MakeAvailable(taskflow)

add_subdirectory(engine)

add_library(conductor_lib INTERFACE)

target_link_libraries(conductor_lib
INTERFACE
    conductor_commands
    conductor_core
    conductor_ecs
    conductor_events
    conductor_plugins
    conductor_runtime
    conductor_scheduler
    conductor_simulation
)

add_subdirectory(apps)
add_subdirectory(benchmarks)
add_subdirectory(examples)
add_subdirectory(tests)