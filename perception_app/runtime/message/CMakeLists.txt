# Message Module CMakeLists.txt

# 设置模块名称
set(MODULE_NAME "message")

# 收集源文件
file(GLOB_RECURSE MESSAGE_SOURCES
    "MessageProtocol.cpp"
)

# 收集头文件
file(GLOB_RECURSE MESSAGE_HEADERS
    "IMessageProtocol.hpp"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 创建静态库
add_library(${MODULE_NAME} STATIC
    ${MESSAGE_SOURCES}
)

# 设置包含目录（使用父模块的公共配置）
target_include_directories(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_INCLUDE_DIRS}
)

# 链接依赖库（使用父模块的公共配置）
target_link_libraries(${MODULE_NAME} PUBLIC
    ${PERCEPTION_COMMON_LIBRARIES}
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
message(STATUS "Message module configuration:")
message(STATUS "  - Sources: ${MESSAGE_SOURCES}")
message(STATUS "  - Headers: ${MESSAGE_HEADERS}")
message(STATUS "  - Using common include dirs: ${PERCEPTION_COMMON_INCLUDE_DIRS}")
message(STATUS "  - Using common libraries: ${PERCEPTION_COMMON_LIBRARIES}")
