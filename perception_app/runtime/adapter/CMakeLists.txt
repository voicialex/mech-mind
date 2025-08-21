# Adapter Module CMakeLists.txt

# 设置模块名称
set(MODULE_NAME "adapter")

# 收集源文件
file(GLOB_RECURSE ADAPTER_SOURCES
    "examples/*.cpp"
    "client_node/*.cpp"
    "master_node/*.cpp"
)

# 收集头文件
file(GLOB_RECURSE ADAPTER_HEADERS
    "examples/*.hpp"
    "client_node/*.hpp"
    "master_node/*.hpp"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# 创建静态库
add_library(${MODULE_NAME} STATIC
    ${ADAPTER_SOURCES}
)

# 设置包含目录（使用父模块的公共配置）
target_include_directories(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_INCLUDE_DIRS}
)

# 链接依赖库（使用父模块的公共配置和communication模块）
target_link_libraries(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_LIBRARIES}
    communication message
)

# 设置编译选项（使用父模块的公共配置）
target_compile_features(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_COMPILE_FEATURES}
)

# 设置编译定义（使用父模块的公共配置）
target_compile_definitions(${MODULE_NAME} PRIVATE
    ${PERCEPTION_COMMON_COMPILE_DEFINITIONS}
)

# 设置输出目录
set_target_properties(${MODULE_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建示例可执行文件
add_executable(adapter_client_example examples/client_node_example.cpp)
target_link_libraries(adapter_client_example adapter communication message perception_app_lib)

add_executable(adapter_server_example examples/master_node_example.cpp)
target_link_libraries(adapter_server_example adapter communication message perception_app_lib)

# 设置示例程序属性
set_target_properties(adapter_client_example adapter_server_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 打印配置信息
message(STATUS "Adapter module configuration:")
message(STATUS "  - Sources: ${ADAPTER_SOURCES}")
message(STATUS "  - Headers: ${ADAPTER_HEADERS}")
message(STATUS "  - Using common include dirs: ${PERCEPTION_COMMON_INCLUDE_DIRS}")
message(STATUS "  - Using common libraries: ${PERCEPTION_COMMON_LIBRARIES}")
