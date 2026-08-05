add_executable(conductor_editor
    src/main.cpp
    src/EditorApplication.cpp
)

target_link_libraries(conductor_editor
PRIVATE
    conductor_core
)

target_include_directories(conductor_editor
PRIVATE
    src
)