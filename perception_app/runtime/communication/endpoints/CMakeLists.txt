# Endpoint Module CMakeLists.txt

# 设置模块名称
set(MODULE_NAME "endpoint")

# 查找依赖库
find_package(asio REQUIRED)
find_package(nlohmann_json REQUIRED)

# 收集源文件
file(GLOB_RECURSE ENDPOINT_SOURCES
    "services/*.cpp"
    "clients/*.cpp"
    "servers/*.cpp"
    "adapters/*.cpp"
    "factories/*.cpp"
)

# 示例程序源文件
set(ENDPOINT_EXAMPLES_SOURCES
    "examples/endpoint_client_example.cpp"
    "examples/endpoint_server_example.cpp"
)

# 收集头文件
file(GLOB_RECURSE ENDPOINT_HEADERS
    "interfaces/*.hpp"
    "services/*.hpp"
    "clients/*.hpp"
    "servers/*.hpp"
    "adapters/*.hpp"
    "factories/*.hpp"
)

# 创建静态库
add_library(${MODULE_NAME} STATIC
    ${ENDPOINT_SOURCES}
)

# 设置包含目录
target_include_directories(${MODULE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# 链接依赖库
target_link_libraries(${MODULE_NAME} PUBLIC
    asio::asio
    nlohmann_json::nlohmann_json
    communication
)

# 设置编译选项
target_compile_features(${MODULE_NAME} PUBLIC
    cxx_std_17
)

# 设置编译定义
target_compile_definitions(${MODULE_NAME} PRIVATE
    ASIO_STANDALONE
)

# 设置输出目录
set_target_properties(${MODULE_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建示例可执行文件
add_executable(endpoint_client_example examples/endpoint_client_example.cpp)
target_link_libraries(endpoint_client_example endpoint)

add_executable(endpoint_server_example examples/endpoint_server_example.cpp)
target_link_libraries(endpoint_server_example endpoint)

# 设置示例程序属性
set_target_properties(endpoint_client_example endpoint_server_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 打印配置信息
message(STATUS "Endpoint module configuration:")
message(STATUS "  - Sources: ${ENDPOINT_SOURCES}")
message(STATUS "  - Headers: ${ENDPOINT_HEADERS}")
message(STATUS "  - Examples: ${ENDPOINT_EXAMPLES_SOURCES}")
