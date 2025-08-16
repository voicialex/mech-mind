cmake_minimum_required(VERSION 3.16)

# 设备中心子模块
project(DeviceCenter VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 功能开关选项
option(BUILD_DEVICE_CENTER "Build Device Center module" ON)
option(BUILD_DEVICE_SERVER "Build Device Server application" ON)
option(BUILD_DEVICE_CLIENT "Build Device Client application" ON)
option(BUILD_DEVICE_TESTS "Build Device Center tests" OFF)

if(NOT BUILD_DEVICE_CENTER)
    message(STATUS "Device Center module disabled")
    return()
endif()

message(STATUS "Building Device Center module...")

# 线程库
find_package(Threads REQUIRED)

# 设置包含目录（自动继承上层的include_directories设置）
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # 只需要添加自己的特定路径
)

# 获取父目录的MechEyeApi库路径
if(DEFINED MECHEYEAPI_LIBRARY_DIRS)
    set(DEVICE_CENTER_MECHEYEAPI_LIBRARY_DIRS ${MECHEYEAPI_LIBRARY_DIRS})
else()
    set(DEVICE_CENTER_MECHEYEAPI_LIBRARY_DIRS "/home/seb/workspace/oss/mech-mind/lib")
endif()

# 源文件
set(DEVICE_CENTER_SOURCES
    src/DeviceServer.cpp
    src/DeviceClient.cpp
    src/DeviceUtils.cpp
)

set(SERVER_APP_SOURCES
    src/server_app.cpp
)

set(CLIENT_APP_SOURCES
    src/client_app.cpp
)

# 创建设备中心库
add_library(device_center STATIC ${DEVICE_CENTER_SOURCES})

# 设置库的属性
set_target_properties(device_center PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER include/DeviceServer.hpp include/DeviceClient.hpp include/DeviceTypes.hpp
)

# 链接库
target_link_libraries(device_center
    Threads::Threads
    perception_app_lib
)

# 设置编译定义
target_compile_definitions(device_center PRIVATE
    DEVICE_CENTER_EXPORTS
)

# 创建服务器应用程序
if(BUILD_DEVICE_SERVER)
    add_executable(device_server ${SERVER_APP_SOURCES})
    
    # 链接库
    target_link_libraries(device_server
        device_center
        perception_app_lib
        Threads::Threads
    )
    
    # 设置编译定义
    target_compile_definitions(device_server PRIVATE
        BUILD_DEVICE_SERVER
    )
    
    # 设置RPATH
    set_target_properties(device_server PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${DEVICE_CENTER_MECHEYEAPI_LIBRARY_DIRS}"
        BUILD_WITH_INSTALL_RPATH FALSE
        SKIP_BUILD_RPATH FALSE
    )
endif()

# 创建客户端应用程序
if(BUILD_DEVICE_CLIENT)
    add_executable(device_client ${CLIENT_APP_SOURCES})
    
    # 链接库
    target_link_libraries(device_client
        device_center
        perception_app_lib
        Threads::Threads
    )
    
    # 设置编译定义
    target_compile_definitions(device_client PRIVATE
        BUILD_DEVICE_CLIENT
    )
    
    # 设置RPATH
    set_target_properties(device_client PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${DEVICE_CENTER_MECHEYEAPI_LIBRARY_DIRS}"
        BUILD_WITH_INSTALL_RPATH FALSE
        SKIP_BUILD_RPATH FALSE
    )
endif()

# 安装规则
install(TARGETS device_center
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(BUILD_DEVICE_SERVER)
    install(TARGETS device_server
        RUNTIME DESTINATION bin
    )
endif()

if(BUILD_DEVICE_CLIENT)
    install(TARGETS device_client
        RUNTIME DESTINATION bin
    )
endif()

install(FILES include/DeviceServer.hpp include/DeviceClient.hpp include/DeviceTypes.hpp
    DESTINATION include/device_center
)

install(DIRECTORY config/
    DESTINATION etc/device_center
    FILES_MATCHING PATTERN "*.json"
)

# 创建日志目录
install(CODE "file(MAKE_DIRECTORY \${CMAKE_INSTALL_PREFIX}/var/log/device_center)")

# 测试
if(BUILD_DEVICE_TESTS)
    enable_testing()
    
    # 添加测试可执行文件
    add_executable(test_device_center
        tests/test_device_center.cpp
    )
    
    target_link_libraries(test_device_center
        device_center
        gtest
        gtest_main
    )
    
    add_test(NAME DeviceCenterTest COMMAND test_device_center)
endif()

# 打印配置信息
message(STATUS "DeviceCenter Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Server: ${BUILD_DEVICE_SERVER}")
message(STATUS "  Build Client: ${BUILD_DEVICE_CLIENT}")
message(STATUS "  Build Tests: ${BUILD_DEVICE_TESTS}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
