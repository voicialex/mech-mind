#pragma once

#include "area_scan_3d_camera/Camera.h"
#include <string>
#include <opencv2/opencv.hpp>

struct FrameSet
{
    mmind::eye::Frame2DAnd3D frame2DAnd3D;

    // 图像数据
    cv::Mat color; // .png格式
    cv::Mat depthImage; // .tiff格式
    cv::Mat renderDepth; // .tiff格式

    // pointCloud, .ply格式
    // 无纹理点云,只包含点的三维坐标 (X, Y, Z)
    // 适用场景：只需要几何形状信息的应用，如物体尺寸测量、碰撞检测等
    mmind::eye::PointCloud pointCloud;
    // 带法线的无纹理点云,包含点的三维坐标 (X, Y, Z) 和法线 (Nx, Ny, Nz)
    // 适用场景：需要表面方向信息的应用，如曲面重建、光照模拟、机器人抓取规划
    mmind::eye::PointCloudWithNormals pointCloudWithNormals;
    // 有纹理点云,包含点的三维坐标 (X, Y, Z) 和颜色信息 (R, G, B)
    // 适用场景：需要颜色信息的应用，如物体识别、场景重建、AR/VR可视化
    mmind::eye::TexturedPointCloud texturedPointCloud;
    // 带法线的有纹理点云,包含点的三维坐标 (X, Y, Z) 和法线 (Nx, Ny, Nz) 和颜色信息 (R, G, B)
    // 适用场景：需要颜色信息的应用，如物体识别、场景重建、AR/VR可视化
    mmind::eye::TexturedPointCloudWithNormals texturedPointCloudWithNormals;

    // 从深度图转换的点云, .ply格式
    mmind::eye::PointCloud pointCloudFromDepth;

    std::string suffix;

    // 数据可用标志
    bool hasColor = false;
    bool hasDepth = false;
    bool hasRenderDepth = false;
    bool hasPointCloud = false;
    bool hasPointCloudWithNormals = false;
    bool hasTexturedPointCloud = false;
    bool hasTexturedPointCloudWithNormals = false;
    bool hasPointCloudFromDepth = false;

    FrameSet(const mmind::eye::Frame2DAnd3D& frame, const std::string& suffix);

    void DecodeFrame();

    cv::Mat renderDepthData(const cv::Mat& depth);

    void processPointCloud(mmind::eye::FrameTransformation transformation);

    void convertDepthToPointCloud(const mmind::eye::DepthMap& depth, const mmind::eye::CameraIntrinsics& intrinsics, mmind::eye::PointCloud& pointCloud);
};
