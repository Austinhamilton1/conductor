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

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/Catch2)

add_subdirectory(engine)
add_subdirectory(apps)
add_subdirectory(tests)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
    --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)