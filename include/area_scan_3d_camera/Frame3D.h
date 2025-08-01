/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2016-2025, Mech-Mind Robotics Technologies Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Info:  https://www.mech-mind.com/
 *
 ******************************************************************************/

#pragma once
#include <memory>
#include "api_global.h"
#include "Array2D.h"
#include "ErrorStatus.h"
#include "CommonTypes.h"

namespace mmind {

namespace eye {

/**
 * @brief Represents a point in @ref DepthMap with the depth (z-coordinate) information.
 */
struct PointZ
{
    float z{0}; ///< Depth channel, default unit: mm, invalid data: nan.
};

/**
 * @brief The pixel element struct with the normal vector coordinate (x, y, z) information.
 */
struct NormalVector
{
    float x{0};         ///< Normal vector X coordinate, invalid data: nan.
    float y{0};         ///< Normal vector Y coordinate, invalid data: nan.
    float z{0};         ///< Normal vector Z coordinate, invalid data: nan.
    float curvature{0}; ///< Normal curvature.
};

/**
 * @brief The pixel element struct in @ref UntexturedPointCloudWithNormals with the coordinate (x,
 * y, z) and normal vector coordinate (x, y, z, curvature) information.
 */
struct PointXYZWithNormals
{
    PointXYZ point;
    NormalVector normal;
};

using DepthMap = Array2D<PointZ>;
using UntexturedPointCloud = Array2D<PointXYZ>;
using UntexturedPointCloudWithNormals = Array2D<PointXYZWithNormals>;
using PointCloud = UntexturedPointCloud;
using PointCloudWithNormals = UntexturedPointCloudWithNormals;

/**
 * @brief Represents the 3D capture result, which can be obtained by calling
 * @ref Camera.capture3D. The 3D data can be in the
 * form of @ref PointZ or @ref PointXYZ and is stored in a 2D array, with each element in the array
 * representing a point. The class also provides methods for saving the point cloud data to a local
 * file.
 *
 */
class MMIND_API_EXPORT Frame3D
{
public:
    /**
     * @brief Constructor.
     */
    Frame3D();

    /**
     * @brief Destructor.
     */
    ~Frame3D();

    /**
     * @brief Copy constructor.
     */
    Frame3D(const Frame3D& other) noexcept;

    /**
     * @brief Copy assignment.
     */
    Frame3D& operator=(const Frame3D& other) noexcept;

    /**
     * @brief Gets the image size of @ref Frame3D.
     */
    Size imageSize() const;

    /**
     * @brief The ID of the @ref Frame3D frame.
     */
    uint64_t frameId() const;

    /**
     * @brief Judges whether @ref Frame3D is empty.
     */
    bool isEmpty() const;

    /**
     * @brief Clears the data in @ref Frame3D and releases the associated resources.
     */
    void clear();

    /**
     * @brief Gets the organized 3D data map with a @ref PointXYZ pixel data format. Each point
     * in @ref PointCloud contains the X, Y, and Z data represented in the camera coordinate
     * system. The X, Y, and Z data default unit is in mm (You can change the unit by calling @ref
     * Camera.setPointCloudUnit before calling @ref Camera.capture3D), and invalid data is in nan.
     * @return See @ref PointCloud for details.
     */
    PointCloud getUntexturedPointCloud() const;

    /**
     * @brief Gets the organized 3D data map with a @ref PointXYZWithNormals pixel data format.
     * Each point in @ref PointCloudWithNormals contains the X, Y, Z data represented in the camera
     * coordinate system. The X, Y, and Z data default unit is in mm (You can change the unit by
     * calling @ref Camera.setPointCloudUnit before calling @ref Camera.capture3D), and invalid data
     * is in nan. Normal vectors are computed for each point. If the @ref Frame3D is obtained by
     * @ref Camera.capture3DWithNormal, the normal vectors are computed at the camera side.
     * @return See @ref PointCloudWithNormals for details.
     */
    PointCloudWithNormals getUntexturedPointCloudWithNormals() const;

    /**
     * @brief Gets the organized 3D data with the @ref PointZ data format. Each point in
     * @ref DepthMap contains the depth (z-coordinate) data in the camera reference frame.
     * If the application only requires the depth data, obtaining the @ref DepthMap is more
     * efficient. The default unit of the depth data is millimeter (You can change the unit by
     * calling @ref Camera.setPointCloudUnit before calling @ref Camera.capture3D), and the invalid
     * data is represented by "nan".
     * @return See @ref DepthMap for details.
     */
    DepthMap getDepthMap() const;

    /**
     * @brief Obtains the 3D data with the @ref PointZ data format. The 3D data is
     * projected under the orthographic projection camera model. Four parameters are provided to
     * assist in converting depth pixel positions into coordinate values in the camera
     * reference frame according to the following equation: X = col * xScale + xOffset, Y = row *
     * yScale + yOffset.
     * @param [out] xScale The output parameter.
     * @param [out] yScale The output parameter.
     * @param [out] xOffset The output parameter.
     * @param [out] yOffset The output parameter.
     * @return See @ref DepthMap for details.
     */
    DepthMap getOrthogonalDepthMap(double& xScale, double& yScale, double& xOffset,
                                   double& yOffset) const;

    /**
     * @brief Saves the untextured point cloud to a file of the input file format and filename.
     * Each point contains x, y, and z fields. The default unit of the coordinate data is
     * millimeter (You can change the unit by calling @ref Camera.setPointCloudUnit before calling
     * @ref Camera.capture3D). Point cloud can be saved in organized or unorganized structure with
     * ACSII mode.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    ErrorStatus saveUntexturedPointCloud(FileFormat fileFormat, const std::string& fileName,
                                         bool isOrganized = false) const;

    /**
     * @brief Saves the untextured point cloud to a file of the input file format and filename.
     * Each point contains x, y, z, normalX, normalY, and normalZ fields. The default unit of the
     * coordinate data is millimeter (You can change the unit by calling @ref
     * Camera.setPointCloudUnit before calling @ref Camera.capture3D). Point cloud can be saved in
     * organized or unorganized structure with ACSII mode.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    ErrorStatus saveUntexturedPointCloudWithNormals(FileFormat fileFormat,
                                                    const std::string& fileName,
                                                    bool isOrganized = false) const;

    /**
     * @brief Saves the untextured point cloud to a file of the input file format and filename.
     * Each point contains x, y, and z fields. The default unit of the coordinate data is
     * millimeter. Point cloud can be saved in organized or unorganized structure with ACSII mode.
     * @param [in] pointCloud The point cloud data to be saved. See @ref PointCloud for details.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @param [in] pointCloudUnit The unit of the coordinate data.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    static ErrorStatus savePointCloud(const PointCloud& pointCloud, FileFormat fileFormat,
                                      const std::string& fileName, bool isOrganized = false,
                                      CoordinateUnit pointCloudUnit = CoordinateUnit::Millimeter);

    /**
     * @brief Saves the untextured point cloud to a file of the input file format and filename.
     * Each point contains x, y, z, normalX, normalY, and normalZ fields. The default unit of the
     * coordinate data is millimeter. Point cloud can be saved in organized or unorganized structure
     * with ACSII mode.
     * @param [in] pointCloud The point cloud data to be saved. See @ref PointCloudWithNormals for
     * details.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @param [in] pointCloudUnit The unit of the coordinate data.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    static ErrorStatus savePointCloudWithNormals(
        const PointCloudWithNormals& pointCloud, FileFormat fileFormat, const std::string& fileName,
        bool isOrganized = false, CoordinateUnit pointCloudUnit = CoordinateUnit::Millimeter);

private:
    friend class CameraImpl;
    friend class Frame2DAnd3D;
    friend class InternalInterfaces;

    std::shared_ptr<class Frame3DImpl> _impl;
    Frame3D(std::shared_ptr<Frame3DImpl>& frameImpl);
};

} // namespace eye

} // namespace mmind
