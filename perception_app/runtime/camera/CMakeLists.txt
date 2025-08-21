# Camera 子模块 CMakeLists.txt

# 源文件定义
set(CAMERA_SOURCES
    CameraManager.cpp
    CameraInfo.cpp
    FrameSet.cpp
    InferenceManager.cpp
    utils/CVWindow.cpp
)

set(CAMERA_HEADERS
    CameraManager.hpp
    CameraInfo.hpp
    FrameSet.hpp
    InferenceManager.hpp
    utils/CVWindow.hpp
    utils/UtilHelper.h
)

# 创建 camera 静态库
add_library(camera STATIC ${CAMERA_SOURCES})

# 设置库属性
set_target_properties(camera PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 设置包含目录
target_include_directories(camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# 设置编译特性
target_compile_features(camera PRIVATE cxx_std_17)

# 设置编译定义
target_compile_definitions(camera PRIVATE
    $<$<BOOL:${USE_HALCON}>:USE_HALCON>
    $<$<BOOL:${USE_OPENCV}>:USE_OPENCV>
)

# 链接依赖库
target_link_libraries(camera
    ${MECHEYEAPI_LIBRARIES}
    $<$<BOOL:${USE_OPENCV}>:opencv_core;opencv_imgproc;opencv_highgui;opencv_imgcodecs>
    $<$<BOOL:${USE_HALCON}>:${Halcon_LIBRARIES}>
)

# 打印构建信息
message(STATUS "Camera module configuration:")
message(STATUS "  - Sources: ${CAMERA_SOURCES}")
message(STATUS "  - Headers: ${CAMERA_HEADERS}")
message(STATUS "  - Using common include dirs: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  - Using common libraries: ${MECHEYEAPI_LIBRARIES}")
