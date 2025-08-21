# Communication Module CMakeLists.txt

# 设置模块名称
set(MODULE_NAME "communication")

# 收集源文件
file(GLOB_RECURSE COMMUNICATION_SOURCES
    "transports/*.cpp"
    "discovery/*.cpp"
    "endpoints/services/*.cpp"
    "endpoints/clients/*.cpp"
    "endpoints/servers/*.cpp"
)

# 收集头文件
file(GLOB_RECURSE COMMUNICATION_HEADERS
    "interfaces/*.hpp"
    "transports/*.hpp"
    "discovery/*.hpp"
    "endpoints/services/*.hpp"
    "endpoints/clients/*.hpp"
    "endpoints/servers/*.hpp"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/discovery
    ${CMAKE_CURRENT_SOURCE_DIR}/endpoints/clients
    ${CMAKE_CURRENT_SOURCE_DIR}/endpoints/servers
    ${CMAKE_CURRENT_SOURCE_DIR}/endpoints/services
    ${CMAKE_CURRENT_SOURCE_DIR}/transports
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# 创建静态库
add_library(${MODULE_NAME} STATIC
    ${COMMUNICATION_SOURCES}
)

# 设置包含目录（使用父模块的公共配置）
target_include_directories(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_INCLUDE_DIRS}
)

# 链接依赖库（使用父模块的公共配置）
target_link_libraries(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_LIBRARIES}
    message  # 链接 message 模块
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

# 打印配置信息
message(STATUS "Communication module configuration:")
message(STATUS "  - Sources: ${COMMUNICATION_SOURCES}")
message(STATUS "  - Headers: ${COMMUNICATION_HEADERS}")
message(STATUS "  - Using common include dirs: ${PERCEPTION_COMMON_INCLUDE_DIRS}")
message(STATUS "  - Using common libraries: ${PERCEPTION_COMMON_LIBRARIES}")
