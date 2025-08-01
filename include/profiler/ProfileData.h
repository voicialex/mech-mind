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
#include "api_global.h"
#include "BatchArray.h"
#include "CommonTypes.h"
#include "ErrorStatus.h"

namespace mmind {
namespace eye {

class ProfileBatchImpl;

/**
 * @brief Describes a single profile.
 */
struct Profile
{
    unsigned int profileIndex{0};            ///< Index of the profile.
    unsigned int encoder{0};                 ///< Corresponding encoder value of the profile.
    const unsigned char* intensity{nullptr}; ///< Pointer to the intensity values of the profile.
    const float* depth{nullptr}; ///< Pointer to the depth values of the profile. The unit of depth
                                 ///< data is millimeter.
};

/**
 * @brief Represents a point in @ref ProfileBatch::TexturedPointCloud with the coordinate (x, y, z,
 * intensity) information.
 */
struct PointXYZI
{
    float x{0};                 ///< X channel, default unit: mm, invalid data: nan.
    float y{0};                 ///< Y channel, default unit: mm, invalid data: nan.
    float z{0};                 ///< Z channel, default unit: mm, invalid data: nan.
    unsigned char intensity{0}; ///< intensity channel.
};

/**
 * @brief Represents a batch of profiles, which can be obtained by calling @ref
 * Profiler.retrieveBatchData(). It contains four elements of profile index, encoder value,
 * intensity image, and depth map.
 */
class MMIND_API_EXPORT ProfileBatch
{
public:
    /**
     * @brief Describes the status of the ProfileBatch object.
     */
    enum class BatchFlag {
        Success =
            0, ///< All profiles in the ProfileBatch object contain valid intensity and depth data.
        Incomplete = 0x1, ///< Some profiles in the ProfileBatch object do not contain valid
                          ///< intensity and depth data.
    };

    using ProfileIndexArray = BatchArray<unsigned int>;
    using EncoderArray = BatchArray<unsigned int>;
    using IntensityImage = BatchArray<unsigned char>;
    using DepthMap = BatchArray<float>;
    using UntexturedPointCloud = BatchArray<PointXYZ>;
    using PointCloud = UntexturedPointCloud;
    using TexturedPointCloud = BatchArray<PointXYZI>;

    /**
     * @brief Constructor.
     */
    ProfileBatch(size_t width);

    /**
     * @brief Default destructor.
     */
    ~ProfileBatch() = default;

    /**
     * @brief Returns the width of the ProfileBatch object (the number of data points per profile).
     */
    size_t width() const;

    /**
     * @brief Returns the height of the ProfileBatch object (the number of profiles in the batch).
     */
    size_t height() const;

    /**
     * @brief Returns the valid height of the ProfileBatch object (the number of profiles with
     * valid intensity and depth data in the batch).
     */
    size_t validHeight() const;

    /**
     * @brief Checks if the ProfileBatch object has no elements.
     */
    bool isEmpty() const;

    /**
     * @brief Reserves the input height for the ProfileBatch object.
     */
    void reserve(size_t height);

    /**
     * @brief Appends the data of one ProfileBatch object to another.
     */
    bool append(const ProfileBatch& batch);

    /**
     * @brief Clears the data in the ProfileBatch object.
     */
    void clear();

    /**
     * @brief Gets a profile in the batch by inputting the index of the profile.
     */
    Profile getProfile(size_t profileIndex) const;

    /**
     * @brief Gets an array of indices of all profiles in the batch. Each profile data corresponds
     * to an index.
     */
    ProfileIndexArray getProfileIndexArray() const;

    /**
     * @brief Gets an array of encoder values of all profiles in the batch. Each profile data
     * corresponds to an encoder value.
     */
    EncoderArray getEncoderArray() const;

    /**
     * @brief Gets the intensity image data in the batch. The invalid data of intensity image is 0.
     */
    IntensityImage getIntensityImage() const;

    /**
     * @brief Gets the depth map data in the batch. Each point in DepthMap contains the Z
     * information in the laser profiler coordinate system. The depth data unit is mm, and invalid
     * data is nan.
     */
    DepthMap getDepthMap() const;

    /**
     * @brief Gets the untextured point cloud data in the batch. Each point in UntexturedPointCloud
     * contains the X, Y, and Z information in the laser profiler coordinate system. The space of
     * the X coordinate data is determined by the xResolution argument. The X coordinate data is
     * determined by the column index. The space of the Y coordinate data is determined by
     * yResolution argument. The Y coordinate data is determined by the encoder values if the
     * useEncoderValues argument is set to true and row index otherwise. The depth data unit is mm,
     * and invalid data is nan.
     * @param [in] xResolution determines the space of the X coordinate data.
     * @param [in] yResolution determines the space of the Y coordinate data.
     * @param [in] useEncoderValues determines whether to use the encoder values or row index as the
     * data source for Y coordinate data.
     * @param [in] triggerInterval trigger interval of the encoder values.
     * @param [in] coordinateUnit the coordinate unit of the point cloud.
     * @return See @ref UntexturedPointCloud for details.
     */
    UntexturedPointCloud getUntexturedPointCloud(
        double xResolution, double yResolution, bool useEncoderValues, int triggerInterval,
        CoordinateUnit coordinateUnit = CoordinateUnit::Millimeter) const;

    /**
     * @brief Gets the textured point cloud data in the batch. Each point in TexturedPointCloud
     * contains the X, Y, Z, and Intensity information in the laser profiler coordinate system. The
     * space of the X coordinate data is determined by the xResolution argument. The X coordinate
     * data is determined by the column index. The space of the Y coordinate data is determined by
     * yResolution argument. The Y coordinate data is determined by the encoder values if the
     * useEncoderValues argument is set to true and row index otherwise. The depth data unit is mm,
     * and invalid data is nan.
     * @param [in] xResolution determines the space of the X coordinate data.
     * @param [in] yResolution determines the space of the Y coordinate data.
     * @param [in] useEncoderValues determines whether to use the encoder values or row index as the
     * data source for Y coordinate data.
     * @param [in] triggerInterval trigger interval of the encoder values.
     * @param [in] coordinateUnit the coordinate unit of the point cloud.
     * @return See @ref UntexturedPointCloud for details.
     */
    TexturedPointCloud getTexturedPointCloud(
        double xResolution, double yResolution, bool useEncoderValues, int triggerInterval,
        CoordinateUnit coordinateUnit = CoordinateUnit::Millimeter) const;

    /**
     * @brief Saves the untextured point cloud data in the batch. Each point in UntexturedPointCloud
     * contains the X, Y, and Z information in the laser profiler coordinate system. The space of
     * the X coordinate data is determined by the xResolution argument. The X coordinate data is
     * determined by the column index. The space of the Y coordinate data is determined by
     * yResolution argument. The Y coordinate data is determined by the encoder values if the
     * useEncoderValues argument is set to true and row index otherwise. The depth data unit is mm,
     * and invalid data is nan.
     * @param [in] xResolution determines the space of the X coordinate data.
     * @param [in] yResolution determines the space of the Y coordinate data.
     * @param [in] useEncoderValues determines whether to use the encoder values or row index as the
     * data source for Y coordinate data.
     * @param [in] triggerInterval trigger interval of the encoder values.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] coordinateUnit the coordinate unit of the point cloud.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref ProfileBatch is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    ErrorStatus saveUntexturedPointCloud(double xResolution, double yResolution,
                                         bool useEncoderValues, int triggerInterval,
                                         FileFormat fileFormat, const std::string& fileName,
                                         CoordinateUnit coordinateUnit = CoordinateUnit::Millimeter,
                                         bool isOrganized = false) const;

    /**
     * @brief Saves the textured point cloud data in the batch. Each point in TexturedPointCloud
     * contains the X, Y, Z, and Intensity information in the laser profiler coordinate system. The
     * space of the X coordinate data is determined by the xResolution argument. The X coordinate
     * data is determined by the column index. The space of the Y coordinate data is determined by
     * yResolution argument. The Y coordinate data is determined by the encoder values if the
     * useEncoderValues argument is set to true and row index otherwise. The depth data unit is mm,
     * and invalid data is nan.
     * @param [in] xResolution determines the space of the X coordinate data.
     * @param [in] yResolution determines the space of the Y coordinate data.
     * @param [in] useEncoderValues determines whether to use the encoder values or row index as the
     * data source for Y coordinate data.
     * @param [in] triggerInterval trigger interval of the encoder values.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] coordinateUnit the coordinate unit of the point cloud.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref ProfileBatch is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    ErrorStatus saveTexturedPointCloud(double xResolution, double yResolution,
                                       bool useEncoderValues, int triggerInterval,
                                       FileFormat fileFormat, const std::string& fileName,
                                       CoordinateUnit coordinateUnit = CoordinateUnit::Millimeter,
                                       bool isOrganized = false) const;

    /**
     * @brief Saves the untextured point cloud data in the batch. Each point in UntexturedPointCloud
     * contains the X, Y, and Z information in the laser profiler coordinate system. The depth data
     * unit is mm, and invalid data is nan.
     * @param [in] pointCloud The point cloud to be saved. See @ref UntexturedPointCloud for
     * details.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @param [in] coordinateUnit the coordinate unit of the point cloud.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref ProfileBatch is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    static ErrorStatus saveUntexturedPointCloud(
        const UntexturedPointCloud& pointCloud, FileFormat fileFormat, const std::string& fileName,
        bool isOrganized = false, CoordinateUnit coordinateUnit = CoordinateUnit::Millimeter);

    /**
     * @brief Saves the textured point cloud data in the batch. Each point in TexturedPointCloud
     * contains the X, Y, Z, and Intensity information in the laser profiler coordinate system. The
     * depth data unit is mm, and invalid data is nan.
     * @param [in] pointCloud The point cloud to be saved. See @ref TexturedPointCloud for details.
     * @param [in] fileFormat The format of the point cloud file. Possible values include PLY, PCD,
     * and CSV. See @ref FileFormat for details.
     * @param [in] fileName The filename of the point cloud file.
     * @param [in] isOrganized Whether the point cloud is organized. An organized point cloud saves
     * all points in order, with invalid data as nan, and an unorganized point cloud saves only
     * valid points.
     * @param [in] coordinateUnit the coordinate unit of the point cloud.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR @ref ProfileBatch is empty.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Error occurred while writing the point cloud
     * file.\n
     */
    static ErrorStatus saveTexturedPointCloud(
        const TexturedPointCloud& pointCloud, FileFormat FileFormat, const std::string& fileName,
        bool isOrganized = false, CoordinateUnit coordinateUnit = CoordinateUnit::Millimeter);

    /**
     * @brief Gets the error code and description of the function.
     */
    ErrorStatus getErrorStatus() const;

    /**
     * @brief Gets the flags of the ProfileBatch object. See @ref BatchFlag for details.
     */
    int getFlag() const;

    /**
     * @brief Checks if the @ref BatchFlag value of the ProfileBatch object matches the input value.
     */
    bool checkFlag(BatchFlag flag) const;

private:
    std::shared_ptr<ProfileBatchImpl> _impl;
    friend class ProfilerImpl;
    friend class VirtualProfilerImpl;
};
} // namespace eye
} // namespace mmind
