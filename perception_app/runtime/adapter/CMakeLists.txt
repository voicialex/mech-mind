# =============================================================================
# Adapter Module CMakeLists.txt
# =============================================================================

# 设置模块名称
set(MODULE_NAME "adapter")

# 收集源文件
file(GLOB_RECURSE ADAPTER_SOURCES
    "client_node/*.cpp"
    "master_node/*.cpp"
)

# 收集头文件
file(GLOB_RECURSE ADAPTER_HEADERS
    "client_node/*.hpp"
    "master_node/*.hpp"
)

# 创建静态库
add_library(${MODULE_NAME} STATIC ${ADAPTER_SOURCES})

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
    ${PERCEPTION_COMMON_INCLUDE_DIRS}
)

target_compile_features(${MODULE_NAME} PUBLIC ${PERCEPTION_COMMON_COMPILE_FEATURES})
target_compile_definitions(${MODULE_NAME} PRIVATE ${PERCEPTION_COMMON_COMPILE_DEFINITIONS})

# 链接依赖库
target_link_libraries(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_LIBRARIES}
    communication message
)

# 示例程序已移动到 binary 目录

# 打印模块信息
message(STATUS "Adapter module: ${ADAPTER_SOURCES}")
