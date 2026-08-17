add_executable(
    conductor_component_benchmark
    BenchmarkComponentStorage.cpp
)

target_link_libraries(conductor_component_benchmark
PRIVATE
    conductor_ecs
)