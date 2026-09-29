add_library(drone_sim
SHARED
    Plugin.cpp
)

target_link_libraries(drone_sim
PRIVATE
    conductor_plugin
)

set_target_properties(drone_sim
PROPERTIES
    OUTPUT_NAME "drone_sim"
)