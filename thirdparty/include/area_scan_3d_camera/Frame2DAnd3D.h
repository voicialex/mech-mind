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
#include "Frame2D.h"
#include "Frame3D.h"

namespace mmind {

namespace eye {

/**
 * @brief Represents a point in @ref TexturedPointCloud with the coordinate (x, y, z) and texture
 * (blue, green, red, and transparency) information.
 */

struct PointXYZBGR
{
    PointXYZBGR() : PointXYZBGR(0, 0, 0, 255) {}
    PointXYZBGR(float x, float y, float z) : PointXYZBGR(x, y, z, 0, 0, 0, 255) {}
    PointXYZBGR(uint8_t b, uint8_t g, uint8_t r, uint8_t a) : PointXYZBGR(0, 0, 0, b, g, r, a) {}
    PointXYZBGR(float x, float y, float z, uint8_t b, uint8_t g, uint8_t r, uint8_t a = 255)
        : x(x), y(y), z(z), r(r), g(g), b(b), a(a)
    {
    }
    PointXYZBGR(const PointXYZBGR& p) : PointXYZBGR(p.x, p.y, p.z, p.b, p.g, p.r, p.a) {}

public:
    float x; ///< X channel, default unit: mm, invalid data: nan.
    float y; ///< Y channel, default unit: mm, invalid data: nan.
    float z; ///< Z channel, default unit: mm, invalid data: nan.

    union {
        struct
        {
            uint8_t b; ///< Blue channel.
            uint8_t g; ///< Green channel.
            uint8_t r; ///< Red channel.
            uint8_t a; ///< Transparency channel.
        };
        float rgb;
    };
};

/**
 * @brief The pixel element struct in @ref TexturedPointCloudWithNormals with the coordinate (x, y,
 * z), texture (blue, green, red and transparency), and normal vector coordinate (x, y, z and
 * curvature) information.
 */
struct PointXYZBGRWithNormals
{
    PointXYZBGR colorPoint;
    NormalVector normal;
};

using TexturedPointCloud = Array2D<PointXYZBGR>;
using TexturedPointCloudWithNormals = Array2D<PointXYZBGRWithNormals>;

/**
 * @brief Represents the 2D and 3D capture results, which can be obtained by calling
 * @ref Camera.capture2DAnd3D. The 3D data can be in the
 * form of @ref PointZ or @ref PointXYZ and is stored in a 2D array, with each element in the array
 * representing a point. The class also provides methods for saving the point cloud data to a local
 * file.
 */
class MMIND_API_EXPORT Frame2DAnd3D
{
public:
    /**
     * @brief Constructor.
     */
    Frame2DAnd3D();

    /**
     * @brief Destructor.
     */
    ~Frame2DAnd3D();

    /**
     * @brief Copy constructor.
     */
    Frame2DAnd3D(const Frame2DAnd3D& other) noexcept;

    /**
     * @brief Copy assignment.
     */
    Frame2DAnd3D& operator=(const Frame2DAnd3D& other) noexcept;

    /**
     * @brief Gets the @ref Frame2D data part.
     */
    Frame2D frame2D() const;

    /**
     * @brief Gets the @ref Frame3D data part.
     */
    Frame3D frame3D() const;

    /**
     * @brief Clears the data in @ref Frame2DAnd3D and releases the associated resources.
     */
    void clear();

    /**
     * @brief Gets the 2D and 3D data aligned pixel-to-pixel with the @ref PointXYZBGR data format.
     * Each point in @ref TexturedPointCloud contains the coordinate data in the
     * camera reference frame and texture information. The default unit of the coordinates is
     * millimeter (You can change the unit by calling @ref Camera.setPointCloudUnit before calling
     * @ref Camera.capture2DAnd3D), and the invalid data is represented by "nan".
     * @return See @ref TexturedPointCloud for details.
     */
    TexturedPointCloud getTexturedPointCloud() const;

    /**
     * @brief Gets the pixel-aligned 2D and 3D data with the @ref PointXYZBGRWithNormals pixel data
     * format. Each point in @ref TexturedPointCloudWithNormals contains the X, Y, and Z data
     * represented in the camera coordinate system and the texture information. X, Y, and Z data
     * default unit is in mm (You can change the unit by calling @ref Camera.setPointCloudUnit
     * before calling @ref Camera.capture2DAnd3D), and invalid data is in nan. Normal vectors are
     * computed for each point. If the @ref Frame2DAnd3D is obtained by @ref
     * Camera.capture2DAnd3DWithNormal, the normal vectors are computed at the camera side.
     * @return See @ref TexturedPointCloudWithNormals for details.
     */
    TexturedPointCloudWithNormals getTexturedPointCloudWithNormals() const;

    /**
     * @brief Saves the textured point cloud to a file of the input file format and filename.
     * Each point contains x, y, z, blue, green, and red fields. The default unit of the coordinate
     * data is millimeter (You can change the unit by calling @ref Camera.setPointCloudUnit before
     * calling @ref Camera.capture2DAnd3D). Point cloud can be saved in organized or unorganized
     * structure with ACSII mode.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame2DAnd3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    ErrorStatus saveTexturedPointCloud(FileFormat fileFormat, const std::string& fileName,
                                       bool isOrganized = false) const;

    /**
     * @brief Saves the textured point cloud to a file of the input file format and filename.
     * Each point contains x, y, z, blue, green, red, normalX, normalY, and normalZ fields. The
     * default unit of the coordinate is millimeter (You can change the unit by calling @ref
     * Camera.setPointCloudUnit before calling @ref Camera.capture2DAnd3D). Point cloud can be saved
     * in organized or unorganized structure with ACSII mode.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame2DAnd3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */

    ErrorStatus saveTexturedPointCloudWithNormals(FileFormat fileFormat,
                                                  const std::string& fileName,
                                                  bool isOrganized = false) const;

    /**
     * @brief Saves the textured point cloud to a file of the input file format and filename.
     * Each point contains x, y, z, blue, green, and red fields. The default unit of the coordinate
     * data is millimeter. Point cloud can be saved in organized or unorganized structure with ACSII
     * mode.
     * @param [in] pointCloud The point cloud data to be saved. See @ref TexturedPointCloud for
     * details.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @param [in] pointCloudUnit The unit of the coordinate data.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame2DAnd3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    static ErrorStatus savePointCloud(const TexturedPointCloud& pointCloud, FileFormat fileFormat,
                                      const std::string& fileName, bool isOrganized = false,
                                      CoordinateUnit pointCloudUnit = CoordinateUnit::Millimeter);

    /**
     * @brief Saves the textured point cloud to a file of the input file format and filename.
     * Each point contains x, y, z, blue, green, red, normalX, normalY, and normalZ fields. The
     * default unit of the coordinate data is millimeter. Point cloud can be saved in organized or
     * unorganized structure with ACSII mode.
     * @param [in] pointCloud The point cloud data to be saved. See @ref
     * TexturedPointCloudWithNormals for details.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @param [in] pointCloudUnit The unit of the coordinate data.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref Frame2DAnd3D is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    static ErrorStatus savePointCloudWithNormals(
        const TexturedPointCloudWithNormals& pointCloud, FileFormat fileFormat,
        const std::string& fileName, bool isOrganized = false,
        CoordinateUnit pointCloudUnit = CoordinateUnit::Millimeter);

private:
    std::shared_ptr<class Frame2DAnd3DImpl> _impl;
    friend class CameraImpl;
    friend class InternalInterfaces;
    Frame2DAnd3D(Frame2DAnd3DImpl* frameImpl);
};

} // namespace eye

} // namespace mmind
