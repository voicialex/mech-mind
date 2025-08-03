cmake_minimum_required(VERSION 3.5)

project(perception_app)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/CameraManager.cpp
    ${PROJECT_SOURCE_DIR}/ConfigHelper.cpp
    ${PROJECT_SOURCE_DIR}/utils/CVWindow.cpp
    ${PROJECT_SOURCE_DIR}/FrameSet.cpp
    ${PROJECT_SOURCE_DIR}/CameraInfo.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp)

if(CMAKE_HOST_WIN32)
  find_package(MechEyeApi REQUIRED CONFIG PATHS "$ENV{MECHEYE_DIR}/API")
  if(NOT MechEyeApi_FOUND)
    message(
      FATAL_ERROR "MechEyeApi not found. Please install MechEyeApi first.")
  endif()
elseif(CMAKE_HOST_UNIX)
  find_package(PkgConfig)
  if(NOT PkgConfig_FOUND)
    message(FATAL_ERROR "PkgConfig not found.")
  else()
    pkg_check_modules(MECHEYEAPI REQUIRED MechEyeApi)
    if(NOT MECHEYEAPI_FOUND)
      message(
        FATAL_ERROR "MechEyeApi not found. Please install MechEyeApi first.")
    endif()
  endif()
endif()

# OpenCV_DIR: set as your OpenCV libraries directory; Uncomment next line to set
# OpenCV_DIR manually

# set(OpenCV_DIR  "path to OpenCV directory")
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
  message(
    FATAL_ERROR
      "OpenCV not found. Please point OpenCV_DIR to the directory of your OpenCV installation (containing the file OpenCVConfig.cmake)."
  )
endif()

include_directories(${MECHEYEAPI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
link_directories(${MECHEYEAPI_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${MECHEYEAPI_LIBRARIES} ${OpenCV_LIBS})
