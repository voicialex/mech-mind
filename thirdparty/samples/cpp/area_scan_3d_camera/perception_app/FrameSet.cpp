#include "FrameSet.hpp"
#include "CameraInfo.hpp"
#include <opencv2/opencv.hpp>
#include <area_scan_3d_camera/api_util.h>
#include <area_scan_3d_camera/PointCloudTransformation.h>
#include <area_scan_3d_camera/Frame2DAnd3D.h>
#include <cmath>

FrameSet::FrameSet(const mmind::eye::Frame2DAnd3D& frame, const std::string& suffix)
    :frame2DAnd3D(frame),
     suffix(suffix)
{
    mmind::eye::Color2DImage colorImage = frame.frame2D().getColorImage();
    color = cv::Mat(colorImage.height(), colorImage.width(), CV_8UC3, colorImage.data());
    hasColor = true;

    mmind::eye::DepthMap depthMap = frame.frame3D().getDepthMap();
    depthImage = cv::Mat(depthMap.height(), depthMap.width(), CV_32FC1, depthMap.data());
    hasDepth = true;
}

void FrameSet::DecodeFrame()
{
    renderDepth = renderDepthData(depthImage);
    hasRenderDepth = true;

    processPointCloud(CameraInfo::getInstance().transformation_);

    convertDepthToPointCloud(frame2DAnd3D.frame3D().getDepthMap(), CameraInfo::getInstance().cameraIntrinsics_, pointCloudFromDepth);
    hasPointCloudFromDepth = true;
}

inline bool isApprox0(double d) { return std::fabs(d) <= DBL_EPSILON; }

cv::Mat FrameSet::renderDepthData(const cv::Mat& depth)
{
    if (depth.empty())
        return cv::Mat();
    cv::Mat mask = cv::Mat(depth == depth);
    double minDepthValue, maxDepthValue;
    cv::minMaxLoc(depth, &minDepthValue, &maxDepthValue, nullptr, nullptr, mask);

    cv::Mat depth8U;
    isApprox0(maxDepthValue - minDepthValue)
        ? depth.convertTo(depth8U, CV_8UC1)
        : depth.convertTo(depth8U, CV_8UC1, (255.0 / (minDepthValue - maxDepthValue)),
                          (((maxDepthValue * 255.0) / (maxDepthValue - minDepthValue)) + 1));

    if (depth8U.empty())
        return cv::Mat();

    cv::Mat coloredDepth;
    cv::applyColorMap(depth8U, coloredDepth, cv::COLORMAP_JET);
    coloredDepth.forEach<cv::Vec3b>([&](auto& val, const int* pos) {
        if (!depth8U.ptr<uchar>(pos[0])[pos[1]]) {
            val[0] = 0;
            val[1] = 0;
            val[2] = 0;
        }
    });
    return coloredDepth;
}

void FrameSet::processPointCloud(mmind::eye::FrameTransformation transformation)
{
    pointCloud = mmind::eye::transformPointCloud(transformation, frame2DAnd3D.frame3D().getUntexturedPointCloud());
    hasPointCloud = true;

    pointCloudWithNormals = mmind::eye::transformPointCloudWithNormals(transformation, frame2DAnd3D.frame3D().getUntexturedPointCloud());
    hasPointCloudWithNormals = true;

    texturedPointCloud = mmind::eye::transformTexturedPointCloud(transformation, frame2DAnd3D.getTexturedPointCloud());
    hasTexturedPointCloud = true;

    texturedPointCloudWithNormals = mmind::eye::transformTexturedPointCloudWithNormals(transformation, frame2DAnd3D.getTexturedPointCloud());
    hasTexturedPointCloudWithNormals = true;
}

void FrameSet::convertDepthToPointCloud(const mmind::eye::DepthMap& depth, const mmind::eye::CameraIntrinsics& intrinsics, mmind::eye::PointCloud& pointCloud)
{
    pointCloud.resize(depth.width(), depth.height());

    for (int i = 0; i < depth.width() * depth.height(); i++) {
        const unsigned row = i / depth.width();
        const unsigned col = i - row * depth.width();
        pointCloud[i].z = depth[i].z;
        pointCloud[i].x =
            static_cast<float>(pointCloud[i].z * (col - intrinsics.depth.cameraMatrix.cx) /
                               intrinsics.depth.cameraMatrix.fx);
        pointCloud[i].y =
            static_cast<float>(pointCloud[i].z * (row - intrinsics.depth.cameraMatrix.cy) /
                               intrinsics.depth.cameraMatrix.fy);
    }
}
