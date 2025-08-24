# =============================================================================
# Endpoint Module CMakeLists.txt
# =============================================================================

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
add_library(${MODULE_NAME} STATIC ${ENDPOINT_SOURCES})

# 配置库属性
set_target_properties(${MODULE_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 配置包含目录和编译选项
target_include_directories(${MODULE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${PERCEPTION_COMMON_INCLUDE_DIRS}
)

target_compile_features(${MODULE_NAME} PUBLIC ${PERCEPTION_COMMON_COMPILE_FEATURES})
target_compile_definitions(${MODULE_NAME} PRIVATE
    ${PERCEPTION_COMMON_COMPILE_DEFINITIONS}
    ASIO_STANDALONE
)

# 链接依赖库
target_link_libraries(${MODULE_NAME} PUBLIC
    asio::asio
    nlohmann_json::nlohmann_json
    communication
)

# 创建示例可执行文件（仅在启用示例时）
if(BUILD_EXAMPLES)
    add_executable(endpoint_client_example examples/endpoint_client_example.cpp)
    target_link_libraries(endpoint_client_example endpoint)
    
    add_executable(endpoint_server_example examples/endpoint_server_example.cpp)
    target_link_libraries(endpoint_server_example endpoint)
    
    # 设置示例程序属性
    set_target_properties(endpoint_client_example endpoint_server_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    message(STATUS "Endpoint examples: endpoint_client_example, endpoint_server_example")
endif()

# 打印模块信息
message(STATUS "Endpoint module: ${ENDPOINT_SOURCES}")
