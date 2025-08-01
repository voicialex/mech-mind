/*******************************************************************************
 *BSD 3-Clause License
 *
 *Copyright (c) 2016-2025, Mech-Mind Robotics Technologies Co., Ltd.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#pragma once
#include "MechEyeApi.h"
#include <pcl/point_types.h>
#include <pcl/io/ply_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <vtkOutputWindow.h>
#include <thread>

inline void toPCL(pcl::PointCloud<pcl::PointXYZ>& pointCloud,
                  const mmind::api::PointXYZMap& pointXYZMap)
{
    // write pointcloudXYZ data
    uint32_t size = pointXYZMap.height() * pointXYZMap.width();
    pointCloud.resize(size);

    for (size_t i = 0; i < size; i++) {
        pointCloud[i].x = 0.001 * pointXYZMap[i].x; // mm to m
        pointCloud[i].y = 0.001 * pointXYZMap[i].y; // mm to m
        pointCloud[i].z = 0.001 * pointXYZMap[i].z; // mm to m
    }

    return;
}

inline void toPCL(pcl::PointCloud<pcl::PointXYZRGB>& colorPointCloud,
                  const mmind::api::PointXYZBGRMap& pointXYZBGRMap)
{
    // write pointcloudXYZRGB data
    uint32_t size = pointXYZBGRMap.height() * pointXYZBGRMap.width();
    colorPointCloud.resize(size);

    for (size_t i = 0; i < size; i++) {
        colorPointCloud[i].x = 0.001 * pointXYZBGRMap[i].x; // mm to m
        colorPointCloud[i].y = 0.001 * pointXYZBGRMap[i].y; // mm to m
        colorPointCloud[i].z = 0.001 * pointXYZBGRMap[i].z; // mm to m

        colorPointCloud[i].r = pointXYZBGRMap[i].r;
        colorPointCloud[i].g = pointXYZBGRMap[i].g;
        colorPointCloud[i].b = pointXYZBGRMap[i].b;
    }

    return;
}

inline void viewPCL(const pcl::PointCloud<pcl::PointXYZ>& pointCloud)
{
    vtkOutputWindow::SetGlobalWarningDisplay(0);
    if (pointCloud.empty())
        return;

    pcl::visualization::PCLVisualizer cloudViewer("Cloud Viewer");
    cloudViewer.setShowFPS(false);
    cloudViewer.setBackgroundColor(0, 0, 0);
    cloudViewer.addPointCloud(pointCloud.makeShared());
    cloudViewer.addCoordinateSystem(0.01);
    cloudViewer.addText("Cloud Size: " + std::to_string(pointCloud.size()), 0, 25, 20, 1, 1, 1,
                        "cloudSize");
    cloudViewer.addText("Press r/R to reset camera view point to center.", 0, 0, 16, 1, 1, 1,
                        "help");
    cloudViewer.initCameraParameters();
    while (!cloudViewer.wasStopped()) {
        cloudViewer.spinOnce(20);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

inline void viewPCL(const pcl::PointCloud<pcl::PointXYZRGB>& colorPointCloud)
{
    vtkOutputWindow::SetGlobalWarningDisplay(0);
    if (colorPointCloud.empty())
        return;

    pcl::visualization::PCLVisualizer cloudViewer("Cloud Viewer");
    cloudViewer.setShowFPS(false);
    cloudViewer.setBackgroundColor(0, 0, 0);
    cloudViewer.addPointCloud(colorPointCloud.makeShared());
    cloudViewer.addCoordinateSystem(0.01);
    cloudViewer.addText("Cloud Size: " + std::to_string(colorPointCloud.size()), 0, 25, 20, 1, 1, 1,
                        "cloudSize");
    cloudViewer.addText("Press r/R to reset camera view point to center.", 0, 0, 16, 1, 1, 1,
                        "help");
    cloudViewer.initCameraParameters();
    while (!cloudViewer.wasStopped()) {
        cloudViewer.spinOnce(20);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

inline void savePLY(const mmind::api::PointXYZMap& pointXYZMap, const std::string& path)
{
    pcl::PointCloud<pcl::PointXYZ> pointCloud(pointXYZMap.width(), pointXYZMap.height());
    toPCL(pointCloud, pointXYZMap);
    pcl::PLYWriter writer;
    writer.write(path, pointCloud, true);
    std::cout << "PointCloudXYZ has : " << pointCloud.width * pointCloud.height << " data points."
              << std::endl;
    std::cout << "PointCloudXYZ saved to: " << path << std::endl;
    return;
}

inline void savePLY(const mmind::api::PointXYZBGRMap& pointXYZBGRMap, const std::string& path)
{
    pcl::PointCloud<pcl::PointXYZRGB> pointCloud(pointXYZBGRMap.width(), pointXYZBGRMap.height());
    toPCL(pointCloud, pointXYZBGRMap);
    pcl::PLYWriter writer;
    writer.write(path, pointCloud, true);
    std::cout << "PointCloudXYZRGB has : " << pointCloud.width * pointCloud.height
              << " data points." << std::endl;
    std::cout << "PointCloudXYZRGB saved to: " << path << std::endl;
    return;
}
