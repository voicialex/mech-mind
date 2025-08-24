# =============================================================================
# Binary 可执行文件构建
# =============================================================================

# 主程序 - perception_app
add_executable(perception_app perception_app_main.cpp)

# 设置主程序属性
set_target_properties(perception_app PROPERTIES
    VERSION ${PROJECT_VERSION}
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${MECHEYEAPI_LIBRARY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    SKIP_BUILD_RPATH FALSE
)

# 配置主程序
target_include_directories(perception_app PRIVATE ${PERCEPTION_COMMON_INCLUDE_DIRS})
target_compile_features(perception_app PRIVATE ${PERCEPTION_COMMON_COMPILE_FEATURES})
target_compile_definitions(perception_app PRIVATE ${PERCEPTION_COMMON_COMPILE_DEFINITIONS})
target_link_libraries(perception_app 
    perception_app_lib
    camera
    message
    communication
    adapter
    ${PERCEPTION_COMMON_LIBRARIES}
)

# 客户端示例程序 - client_node_example
add_executable(client_node_example client_node_example.cpp)

# 设置客户端示例程序属性
set_target_properties(client_node_example PROPERTIES
    VERSION ${PROJECT_VERSION}
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${MECHEYEAPI_LIBRARY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    SKIP_BUILD_RPATH FALSE
)

# 配置客户端示例程序
target_include_directories(client_node_example PRIVATE ${PERCEPTION_COMMON_INCLUDE_DIRS})
target_compile_features(client_node_example PRIVATE ${PERCEPTION_COMMON_COMPILE_FEATURES})
target_compile_definitions(client_node_example PRIVATE ${PERCEPTION_COMMON_COMPILE_DEFINITIONS})
target_link_libraries(client_node_example 
    perception_app_lib
    camera
    message
    communication
    adapter
    ${PERCEPTION_COMMON_LIBRARIES}
)

# 服务端示例程序 - master_node_example
add_executable(master_node_example master_node_example.cpp)

# 设置服务端示例程序属性
set_target_properties(master_node_example PROPERTIES
    VERSION ${PROJECT_VERSION}
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${MECHEYEAPI_LIBRARY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    SKIP_BUILD_RPATH FALSE
)

# 配置服务端示例程序
target_include_directories(master_node_example PRIVATE ${PERCEPTION_COMMON_INCLUDE_DIRS})
target_compile_features(master_node_example PRIVATE ${PERCEPTION_COMMON_COMPILE_FEATURES})
target_compile_definitions(master_node_example PRIVATE ${PERCEPTION_COMMON_COMPILE_DEFINITIONS})
target_link_libraries(master_node_example 
    perception_app_lib
    camera
    message
    communication
    adapter
    ${PERCEPTION_COMMON_LIBRARIES}
)

# =============================================================================
# 安装配置
# =============================================================================

# 安装可执行文件
install(TARGETS 
    perception_app
    client_node_example
    master_node_example
    RUNTIME DESTINATION bin
)

# 安装配置文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../config/
    DESTINATION etc/perception_app
    FILES_MATCHING PATTERN "*.json"
)
