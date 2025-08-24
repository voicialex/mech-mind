cmake_minimum_required(VERSION 3.16)

project(perception_app VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# 全局配置和设置
# =============================================================================

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# =============================================================================
# 编译选项配置
# =============================================================================

# 依赖库选项
option(USE_PCL "Enable PCL support" OFF)
option(USE_HALCON "Enable Halcon support" OFF)
option(USE_OPENCV "Enable OpenCV support" ON)

# 构建选项
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build test programs" OFF)
option(ENABLE_VERBOSE "Enable verbose build output" OFF)

# 调试选项
option(ENABLE_DEBUG "Enable debug output" OFF)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_OPTIMIZATION "Enable compiler optimization" ON)

# =============================================================================
# 依赖库配置
# =============================================================================

# 查找 MechEyeApi (必需)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MECHEYEAPI REQUIRED MechEyeApi)
if(NOT MECHEYEAPI_FOUND)
    message(FATAL_ERROR "MechEyeApi not found. Please install MechEyeApi first.")
endif()

# 查找 OpenCV (可选)
if(USE_OPENCV)
    find_package(OpenCV REQUIRED)
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV not found. Please install OpenCV first.")
    endif()
endif()

# 查找 PCL (可选)
if(USE_PCL)
    # 启用 C 语言支持以查找 MPI_C
    enable_language(C)
    find_package(MPI REQUIRED)
    if(MPI_FOUND)
        message(STATUS "MPI found: ${MPI_CXX_LIBRARIES}")
    endif()
    
    # 设置 VTK 的 MPI 配置
    set(VTK_USE_MPI ON)
    set(VTK_MPI_NUMPROCS 1)
    
    find_package(PCL REQUIRED)
    if(NOT PCL_FOUND)
        message(FATAL_ERROR "PCL not found. Please install PCL first.")
    endif()
    add_definitions(${PCL_DEFINITIONS})
endif()

# 查找 Halcon (可选)
if(USE_HALCON)
    find_package(Halcon REQUIRED)
    if(NOT Halcon_FOUND)
        message(FATAL_ERROR "Halcon not found. Please install Halcon first.")
    endif()
endif()

# =============================================================================
# 全局包含目录和库配置
# =============================================================================

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/../thirdparty)

message(STATUS "T====================HIRDPARTY_DIR: ${THIRDPARTY_DIR}")

# 设置包含目录
set(PERCEPTION_COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inference
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${MECHEYEAPI_INCLUDE_DIRS}
    $<$<BOOL:${USE_OPENCV}>:${OpenCV_INCLUDE_DIRS}>
    $<$<BOOL:${USE_PCL}>:${PCL_INCLUDE_DIRS}>
    $<$<BOOL:${USE_HALCON}>:${Halcon_INCLUDE_DIRS}>
)

# 收集目标库
set(PERCEPTION_COMMON_LIBRARIES ${MECHEYEAPI_LIBRARIES})

if(USE_OPENCV)
    list(APPEND PERCEPTION_COMMON_LIBRARIES ${OpenCV_LIBS})
endif()

if(USE_PCL)
    if(MPI_FOUND)
        list(APPEND PERCEPTION_COMMON_LIBRARIES ${MPI_CXX_LIBRARIES})
    endif()
    list(APPEND PERCEPTION_COMMON_LIBRARIES ${PCL_LIBRARIES})
endif()

if(USE_HALCON)
    list(APPEND PERCEPTION_COMMON_LIBRARIES ${Halcon_LIBRARIES})
endif()

# 设置编译特性
set(PERCEPTION_COMMON_COMPILE_FEATURES cxx_std_17)

# 设置编译定义
set(PERCEPTION_COMMON_COMPILE_DEFINITIONS
    $<$<BOOL:${USE_PCL}>:USE_PCL>
    $<$<BOOL:${USE_HALCON}>:USE_HALCON>
    $<$<BOOL:${USE_OPENCV}>:USE_OPENCV>
    $<$<BOOL:${ENABLE_DEBUG}>:DEBUG_MODE>
)

# 设置编译标志
if(ENABLE_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

if(ENABLE_OPTIMIZATION)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    endif()
endif()

if(ENABLE_VERBOSE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# =============================================================================
# 链接器配置
# =============================================================================

# 配置链接器标志（避免库版本冲突）
if(MECHEYEAPI_LIBRARY_DIRS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${MECHEYEAPI_LIBRARY_DIRS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-warn-search-mismatch")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined-version")
endif()

# 添加 thirdparty/lib 到链接器搜索路径
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${THIRDPARTY_DIR}/lib")

# 设置RPATH配置
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/lib;${THIRDPARTY_DIR}/lib")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${THIRDPARTY_DIR}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# =============================================================================
# 核心库构建
# =============================================================================

# 核心源文件
set(CORE_SOURCES
    runtime/configure/ConfigHelper.cpp
    runtime/message/MessageProtocol.cpp
    runtime/message/ProtocolDefinitions.cpp
    inference/ExampleInference.cpp
)

# 创建核心静态库
add_library(perception_app_lib STATIC ${CORE_SOURCES})

# 设置核心库属性
set_target_properties(perception_app_lib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 配置核心库
target_include_directories(perception_app_lib PUBLIC ${PERCEPTION_COMMON_INCLUDE_DIRS})
target_compile_features(perception_app_lib PUBLIC ${PERCEPTION_COMMON_COMPILE_FEATURES})
target_compile_definitions(perception_app_lib PRIVATE ${PERCEPTION_COMMON_COMPILE_DEFINITIONS})
target_link_libraries(perception_app_lib ${PERCEPTION_COMMON_LIBRARIES})

# =============================================================================
# 子模块构建
# =============================================================================

# 添加子模块（按依赖顺序）
add_subdirectory(runtime/camera)
add_subdirectory(runtime/message)
add_subdirectory(runtime/communication)
add_subdirectory(runtime/adapter)

# =============================================================================
# Binary 可执行文件构建
# =============================================================================

# 添加 binary 子目录
add_subdirectory(binary)

# =============================================================================
# 安装配置
# =============================================================================

# 安装核心库
install(TARGETS perception_app_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# =============================================================================
# 构建信息输出
# =============================================================================

# 打印配置信息
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

message(STATUS "=== Dependencies ===")
message(STATUS "PCL support: ${USE_PCL}")
message(STATUS "Halcon support: ${USE_HALCON}")
message(STATUS "OpenCV support: ${USE_OPENCV}")

message(STATUS "=== Build Options ===")
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Enable debug: ${ENABLE_DEBUG}")
message(STATUS "Enable warnings: ${ENABLE_WARNINGS}")
message(STATUS "Enable optimization: ${ENABLE_OPTIMIZATION}")
message(STATUS "Verbose output: ${ENABLE_VERBOSE}")

message(STATUS "=== Dependencies ===")
message(STATUS "MechEyeApi: ${MECHEYEAPI_VERSION}")
if(USE_OPENCV)
    message(STATUS "OpenCV: ${OpenCV_VERSION}")
endif()
if(USE_PCL)
    message(STATUS "PCL: ${PCL_VERSION}")
endif()
if(USE_HALCON)
    message(STATUS "Halcon: ${Halcon_VERSION}")
endif()

message(STATUS "=== Build Summary ===")
message(STATUS "Core sources: ${CORE_SOURCES}")
message(STATUS "Include directories: ${PERCEPTION_COMMON_INCLUDE_DIRS}")
message(STATUS "Common libraries: ${PERCEPTION_COMMON_LIBRARIES}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================")

