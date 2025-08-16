cmake_minimum_required(VERSION 3.16)

project(perception_app VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建类型选项
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 功能开关选项
option(USE_PCL "Enable PCL support" OFF)
option(USE_HALCON "Enable Halcon support" OFF)
option(USE_OPENCV "Enable OpenCV support" ON)
option(BUILD_DEVICE_CENTER "Build Device Center module" ON)

# 打印配置信息
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "PCL support: ${USE_PCL}")
message(STATUS "Halcon support: ${USE_HALCON}")
message(STATUS "OpenCV support: ${USE_OPENCV}")
message(STATUS "Device Center support: ${BUILD_DEVICE_CENTER}")

# 查找 MechEyeApi
find_package(PkgConfig REQUIRED)
pkg_check_modules(MECHEYEAPI REQUIRED MechEyeApi)
if(NOT MECHEYEAPI_FOUND)
    message(FATAL_ERROR "MechEyeApi not found. Please install MechEyeApi first.")
endif()

message(STATUS "MechEyeApi: ${MECHEYEAPI_VERSION}")

# 查找 OpenCV
if(USE_OPENCV)
    find_package(OpenCV REQUIRED)
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV not found. Please install OpenCV first.")
    endif()
    message(STATUS "OpenCV: ${OpenCV_VERSION}")
endif()

# 查找 PCL
if(USE_PCL)
    # 首先查找 MPI，解决 VTK 的 MPI 依赖问题
    # 启用 C 语言支持以查找 MPI_C
    enable_language(C)
    find_package(MPI REQUIRED)
    if(MPI_FOUND)
        message(STATUS "MPI found: ${MPI_CXX_LIBRARIES}")
        message(STATUS "MPI include dirs: ${MPI_CXX_INCLUDE_DIRS}")
        message(STATUS "MPI C libraries: ${MPI_C_LIBRARIES}")
    endif()
    
    # 设置 VTK 的 MPI 配置
    set(VTK_USE_MPI ON)
    set(VTK_MPI_NUMPROCS 1)
    
    find_package(PCL REQUIRED)
    if(NOT PCL_FOUND)
        message(FATAL_ERROR "PCL not found. Please install PCL first.")
    endif()
    message(STATUS "PCL: ${PCL_VERSION}")
    add_definitions(${PCL_DEFINITIONS})
endif()

# 查找 Halcon
if(USE_HALCON)
    find_package(Halcon REQUIRED)
    if(NOT Halcon_FOUND)
        message(FATAL_ERROR "Halcon not found. Please install Halcon first.")
    endif()
    message(STATUS "Halcon: ${Halcon_VERSION}")
endif()

# 设置包含目录
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inference
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${MECHEYEAPI_INCLUDE_DIRS}
    $<$<BOOL:${USE_OPENCV}>:${OpenCV_INCLUDE_DIRS}>
    $<$<BOOL:${USE_PCL}>:${PCL_INCLUDE_DIRS}>
    $<$<BOOL:${USE_HALCON}>:${Halcon_INCLUDE_DIRS}>
)

# 源文件定义
set(SOURCES
    runtime/core/CameraManager.cpp
    runtime/core/utils/CVWindow.cpp
    runtime/core/FrameSet.cpp
    runtime/core/CameraInfo.cpp
    runtime/core/InferenceManager.cpp
    runtime/configure/ConfigHelper.cpp
    runtime/communication/CommunicationManager.cpp
    runtime/communication/CommunicationInterface.cpp
    runtime/message/MessageProtocol.cpp
    inference/ExampleInference.cpp
)

set(MAIN_SOURCES
    runtime/main.cpp
)

# 设置库目录
set(LIBRARY_DIRS ${MECHEYEAPI_LIBRARY_DIRS})

include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS}
    $<$<BOOL:${USE_OPENCV}>:${OpenCV_LIBRARY_DIRS}>
    $<$<BOOL:${USE_PCL}>:${PCL_LIBRARY_DIRS}>
    $<$<BOOL:${USE_HALCON}>:${Halcon_LIBRARY_DIRS}>
)

# 配置链接器标志（避免库版本冲突）
if(MECHEYEAPI_LIBRARY_DIRS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${MECHEYEAPI_LIBRARY_DIRS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${MECHEYEAPI_LIBRARY_DIRS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-warn-search-mismatch")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined-version")
    
    # 设置编译时的库搜索路径
    set(ENV{LD_LIBRARY_PATH} "${MECHEYEAPI_LIBRARY_DIRS}:$ENV{LD_LIBRARY_PATH}")
endif()

# 设置RPATH配置
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${MECHEYEAPI_LIBRARY_DIRS}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# 创建静态库
add_library(perception_app_lib STATIC ${SOURCES})

# 设置库属性
set_target_properties(perception_app_lib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER include/InferenceInterface.hpp
)

# 设置接口属性，供子模块继承
target_include_directories(perception_app_lib INTERFACE ${INCLUDE_DIRS})
target_compile_definitions(perception_app_lib INTERFACE
    $<$<BOOL:${USE_PCL}>:USE_PCL>
    $<$<BOOL:${USE_HALCON}>:USE_HALCON>
    $<$<BOOL:${USE_OPENCV}>:USE_OPENCV>
)

# 创建可执行文件
add_executable(perception_app ${MAIN_SOURCES})

# 设置目标属性
set_target_properties(perception_app PROPERTIES
    VERSION ${PROJECT_VERSION}
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${MECHEYEAPI_LIBRARY_DIRS}"
    BUILD_WITH_INSTALL_RPATH FALSE
    SKIP_BUILD_RPATH FALSE
)

# 设置编译特性
target_compile_features(perception_app_lib PRIVATE cxx_std_17)
target_compile_features(perception_app PRIVATE cxx_std_17)

# 设置编译定义
target_compile_definitions(perception_app_lib PRIVATE
    $<$<BOOL:${USE_PCL}>:USE_PCL>
    $<$<BOOL:${USE_HALCON}>:USE_HALCON>
    $<$<BOOL:${USE_OPENCV}>:USE_OPENCV>
)

target_compile_definitions(perception_app PRIVATE
    $<$<BOOL:${USE_PCL}>:USE_PCL>
    $<$<BOOL:${USE_HALCON}>:USE_HALCON>
    $<$<BOOL:${USE_OPENCV}>:USE_OPENCV>
)

# 收集目标库
set(TARGET_LIBRARIES ${MECHEYEAPI_LIBRARIES})

if(USE_OPENCV)
    list(APPEND TARGET_LIBRARIES ${OpenCV_LIBS})
endif()

if(USE_PCL)
    # 添加 MPI 库
    if(MPI_FOUND)
        list(APPEND TARGET_LIBRARIES ${MPI_CXX_LIBRARIES})
    endif()
    list(APPEND TARGET_LIBRARIES ${PCL_LIBRARIES})
endif()

if(USE_HALCON)
    list(APPEND TARGET_LIBRARIES ${Halcon_LIBRARIES})
endif()

# 链接库
target_link_libraries(perception_app_lib ${TARGET_LIBRARIES})
target_link_libraries(perception_app perception_app_lib)

# 安装配置
install(TARGETS perception_app_lib perception_app
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件
install(FILES include/InferenceInterface.hpp
    DESTINATION include/perception_app
)

# 安装配置文件
install(DIRECTORY config/
    DESTINATION etc/perception_app
    FILES_MATCHING PATTERN "*.json"
)

# 添加Device Center子模块
if(BUILD_DEVICE_CENTER)
    add_subdirectory(device_center)
endif()

# 打印构建信息
message(STATUS "=== Build Summary ===")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Include directories: ${INCLUDE_DIRS}")
message(STATUS "Library directories: ${LIBRARY_DIRS}")
message(STATUS "Target libraries: ${TARGET_LIBRARIES}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================")

