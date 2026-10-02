add_library(drone_sim
SHARED
    Plugin.cpp
)

target_link_libraries(drone_sim
PRIVATE
    conductor_lib
)

set_target_properties(drone_sim
PROPERTIES
    OUTPUT_NAME "drone_sim"
)

add_library(serial_query
SHARED
    SerialQueryPlugin.cpp
)

target_link_libraries(serial_query
PRIVATE
    conductor_lib
)

set_target_properties(serial_query
PROPERTIES
    OUTPUT_NAME "serial_query"
)

add_library(parallel_query
SHARED
    ParallelQueryPlugin.cpp
)

target_link_libraries(parallel_query
PRIVATE
    conductor_lib
)

set_target_properties(parallel_query
PROPERTIES
    OUTPUT_NAME "parallel_query"
)