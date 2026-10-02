add_executable(conductor_component_benchmark
    BenchmarkComponentStorage.cpp
)

add_executable(conductor_query_benchmark
    BenchmarkQuery.cpp
)

target_link_libraries(conductor_component_benchmark
PRIVATE
    conductor_lib
)

target_link_libraries(conductor_query_benchmark
PRIVATE
    conductor_lib
)