cmake_minimum_required(VERSION 3.5)

project(perception_app)

set(CMAKE_CXX_STANDARD 17)

# 构建类型选项
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 功能开关选项
option(USE_PCL "Enable PCL support" OFF)
option(USE_HALCON "Enable Halcon support" OFF)
option(USE_OPENCV "Enable OpenCV support" ON)
option(BUILD_DEVICE_CENTER "Build Device Center module" ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "PCL support: ${USE_PCL}")
message(STATUS "Halcon support: ${USE_HALCON}")
message(STATUS "OpenCV support: ${USE_OPENCV}")
message(STATUS "Device Center support: ${BUILD_DEVICE_CENTER}")

set(SOURCES
    ${PROJECT_SOURCE_DIR}/runtime/CameraManager.cpp
    ${PROJECT_SOURCE_DIR}/runtime/ConfigHelper.cpp
    ${PROJECT_SOURCE_DIR}/runtime/utils/CVWindow.cpp
    ${PROJECT_SOURCE_DIR}/runtime/FrameSet.cpp
    ${PROJECT_SOURCE_DIR}/runtime/CameraInfo.cpp
    ${PROJECT_SOURCE_DIR}/runtime/InferenceManager.cpp
    ${PROJECT_SOURCE_DIR}/inference/ExampleInference.cpp
    ${PROJECT_SOURCE_DIR}/communication/CommunicationManager.cpp
    ${PROJECT_SOURCE_DIR}/communication/CommunicationInterface.cpp
    ${PROJECT_SOURCE_DIR}/runtime/main.cpp)

# 查找 MechEyeApi
if(CMAKE_HOST_WIN32)
  find_package(MechEyeApi REQUIRED CONFIG PATHS "$ENV{MECHEYE_DIR}/API")
  if(NOT MechEyeApi_FOUND)
    message(FATAL_ERROR "MechEyeApi not found. Please install MechEyeApi first.")
  endif()
elseif(CMAKE_HOST_UNIX)
  find_package(PkgConfig)
  if(NOT PkgConfig_FOUND)
    message(FATAL_ERROR "PkgConfig not found.")
  else()
    pkg_check_modules(MECHEYEAPI REQUIRED MechEyeApi)
    if(NOT MECHEYEAPI_FOUND)
      message(FATAL_ERROR "MechEyeApi not found. Please install MechEyeApi first.")
    endif()
  endif()
endif()

message(STATUS "MechEyeApi found: ${MECHEYEAPI_FOUND}")
if(MECHEYEAPI_FOUND)
    message(STATUS "MechEyeApi version: ${MECHEYEAPI_VERSION}")
    message(STATUS "MechEyeApi include dirs: ${MECHEYEAPI_INCLUDE_DIRS}")
    message(STATUS "MechEyeApi library dirs: ${MECHEYEAPI_LIBRARY_DIRS}")
    message(STATUS "MechEyeApi libraries: ${MECHEYEAPI_LIBRARIES}")
endif()

# 查找 OpenCV
if(USE_OPENCV)
  # OpenCV_DIR: set as your OpenCV libraries directory; Uncomment next line to set
  # OpenCV_DIR manually
  # set(OpenCV_DIR  "path to OpenCV directory")
  
  find_package(OpenCV REQUIRED)
  if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please point OpenCV_DIR to the directory of your OpenCV installation (containing the file OpenCVConfig.cmake).")
  endif()
  
  message(STATUS "OpenCV found: ${OpenCV_FOUND}")
  if(OpenCV_FOUND)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV library dirs: ${OpenCV_LIBRARY_DIRS}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
  endif()
endif()

# 查找 PCL (Point Cloud Library)
if(USE_PCL)
  find_package(PCL REQUIRED)
  if(NOT PCL_FOUND)
    message(FATAL_ERROR "PCL not found. Please install PCL first.")
  endif()
  
  message(STATUS "PCL found: ${PCL_FOUND}")
  if(PCL_FOUND)
    message(STATUS "PCL version: ${PCL_VERSION}")
    message(STATUS "PCL include dirs: ${PCL_INCLUDE_DIRS}")
    message(STATUS "PCL library dirs: ${PCL_LIBRARY_DIRS}")
    message(STATUS "PCL libraries: ${PCL_LIBRARIES}")
  endif()
  
  # 添加 PCL 编译定义
  add_definitions(${PCL_DEFINITIONS})
endif()

# 查找 Halcon
if(USE_HALCON)
  find_package(Halcon REQUIRED)
  if(NOT Halcon_FOUND)
    message(FATAL_ERROR "Halcon not found. Please install Halcon first.")
  endif()
  
  message(STATUS "Halcon found: ${Halcon_FOUND}")
  if(Halcon_FOUND)
    message(STATUS "Halcon version: ${Halcon_VERSION}")
    message(STATUS "Halcon include dirs: ${Halcon_INCLUDE_DIRS}")
    message(STATUS "Halcon library dirs: ${Halcon_LIBRARY_DIRS}")
    message(STATUS "Halcon libraries: ${Halcon_LIBRARIES}")
  endif()
endif()

# 设置包含目录
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inference
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${MECHEYEAPI_INCLUDE_DIRS}
)

if(USE_OPENCV)
    list(APPEND INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
endif()

if(USE_PCL)
    list(APPEND INCLUDE_DIRS ${PCL_INCLUDE_DIRS})
endif()

if(USE_HALCON)
    list(APPEND INCLUDE_DIRS ${Halcon_INCLUDE_DIRS})
endif()

include_directories(${INCLUDE_DIRS})

# 设置库目录
set(LIBRARY_DIRS
    ${MECHEYEAPI_LIBRARY_DIRS}
)

if(USE_OPENCV)
    list(APPEND LIBRARY_DIRS ${OpenCV_LIBRARY_DIRS})
endif()

if(USE_PCL)
    list(APPEND LIBRARY_DIRS ${PCL_LIBRARY_DIRS})
endif()

if(USE_HALCON)
    list(APPEND LIBRARY_DIRS ${Halcon_LIBRARY_DIRS})
endif()

link_directories(${LIBRARY_DIRS})

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置目标链接库
set(TARGET_LIBRARIES
    ${MECHEYEAPI_LIBRARIES}
)

if(USE_OPENCV)
    list(APPEND TARGET_LIBRARIES ${OpenCV_LIBS})
endif()

if(USE_PCL)
    list(APPEND TARGET_LIBRARIES ${PCL_LIBRARIES})
endif()

if(USE_HALCON)
    list(APPEND TARGET_LIBRARIES ${Halcon_LIBRARIES})
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} ${TARGET_LIBRARIES})

# 设置编译选项
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# 设置编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_PCL}>:USE_PCL>
    $<$<BOOL:${USE_HALCON}>:USE_HALCON>
    $<$<BOOL:${USE_OPENCV}>:USE_OPENCV>
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装配置文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config/
    DESTINATION config
    FILES_MATCHING PATTERN "*.json"
)

# 安装文档
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README_InferenceInterface.md
    DESTINATION share/doc/${PROJECT_NAME}
)

# 添加Device Center子模块
if(BUILD_DEVICE_CENTER)
    add_subdirectory(device_center)
endif()

# 打印构建信息
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Include directories: ${INCLUDE_DIRS}")
message(STATUS "Library directories: ${LIBRARY_DIRS}")
message(STATUS "Target libraries: ${TARGET_LIBRARIES}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================================")

