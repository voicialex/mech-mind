#pragma once
#include <string>
#include <cmath>
#include <limits>
#include <cstring>
#include "api_global.h"

#define PI (3.14159265358979323846)
namespace mmind {

namespace eye {

/**
 * @brief The unit of the coordinate data of the point cloud.
 */
enum struct CoordinateUnit { Millimeter, Meter };

enum struct FileFormat {
    PLY,
    PCD,
    CSV,
};

/**
 * @brief Describes a value range.
 */
template <typename T>
struct Range
{
    constexpr Range() : min(0), max(0) {}
    constexpr Range(T min, T max) : min(min), max(max) {}
    T min;
    T max;
};

/**
 * @brief Describes a two-dimensional size with a width and a height.
 */
struct Size
{
    constexpr Size() : width(0), height(0) {}
    constexpr Size(size_t width, size_t height) : width(width), height(height) {}
    bool operator==(const Size& other) { return width == other.width && height == other.height; }

    size_t width;
    size_t height;
};

/**
 * @brief Describes a two-dimensional size with a width and a height using double-precision
 * floating-point numbers.
 */
struct MMIND_API_EXPORT SizeF
{
    constexpr SizeF() : width(0), height(0) {}
    constexpr SizeF(double width, double height) : width(width), height(height) {}
    bool operator==(const SizeF& other);

    double width;
    double height;
};

/**
 * @brief Describes a region of interest (ROI).
 */
struct ROI
{
    constexpr ROI() : upperLeftX(0), upperLeftY(0), width(0), height(0) {}
    constexpr ROI(unsigned upperLeftX, unsigned upperLeftY, size_t width, size_t height)
        : upperLeftX(upperLeftX), upperLeftY(upperLeftY), width(width), height(height)
    {
    }
    bool operator==(const ROI& other) { return width == other.width && height == other.height; }

    unsigned upperLeftX; ///< The column coordinate of the upper-left corner of the ROI.
    unsigned upperLeftY; ///< The row coordinate of the upper-left corner of the ROI.
    size_t width;
    size_t height;
};

/**
 * @brief Describes the region of interest (ROI) of a laser profiler.
 */
struct MMIND_API_EXPORT ProfileROI
{
    constexpr ProfileROI() : xAxisCenter(0), width(0), height(0) {}
    constexpr ProfileROI(double xAxisCenter, double width, double height)
        : xAxisCenter(xAxisCenter), width(width), height(height)
    {
    }
    bool operator==(const ProfileROI& other);

    double xAxisCenter; ///< The position (in mm) of the ROI's center on the X-axis.
    double width;       ///< The X-axis width (in mm) of the ROI.
    double height;      ///< The Z-axis height (in mm) of the ROI.
};

/**
 * @brief Represents a point in @ref UntexturedPointCloud with the coordinate (x, y, z) information.
 */
struct PointXYZ
{
    float x{0}; ///< X channel, default unit: mm, invalid data: nan.
    float y{0}; ///< Y channel, default unit: mm, invalid data: nan.
    float z{0}; ///< Z channel, default unit: mm, invalid data: nan.
};

/**
 * @brief Defines the rigid body transformations from one reference frame to another, including the
 * rotation matrix and translation vector.
 */
struct FrameTransformation
{
public:
    FrameTransformation() = default;
    FrameTransformation(double rotateX, double rotateY, double rotateZ, double translateX,
                        double translateY, double translateZ)
    {
        rotate(rotateX, Axis::X);
        rotate(rotateY, Axis::Y);
        rotate(rotateZ, Axis::Z);
        translate(translateX, translateY, translateZ);
    }

    FrameTransformation(const FrameTransformation& rhs)
    {
        memcpy(&rotation, &rhs.rotation, 9 * sizeof(double));
        memcpy(&translation, &rhs.translation, 3 * sizeof(double));
    }

    FrameTransformation& operator=(const FrameTransformation& rhs)
    {
        if (this == &rhs)
            return *this;

        memcpy(&rotation, &rhs.rotation, 9 * sizeof(double));
        memcpy(&translation, &rhs.translation, 3 * sizeof(double));
        return *this;
    }

    enum class Axis { X, Y, Z };

    /**
     * @brief Rotates the reference frame of the point cloud.
     * @param [in]  theta The amount of rotation.
     * @param [in]  rotationAxis  The axis around which the rotation is
     * performed. X-, Y-, and Z-Axes can be input.
     */
    void rotate(double theta, Axis rotationAxis)
    {
        const double radians{theta * PI / 180.0};
        const double cosValue{cos(radians)};
        const double sinValue{sin(radians)};

        switch (rotationAxis) {
        case Axis::X:
        {
            double rotationXMatrix[3][3] = {
                {1, 0, 0}, {0, cosValue, -sinValue}, {0, sinValue, cosValue}};
            multiMatrix(rotationXMatrix);
        } break;
        case Axis::Y:
        {
            double rotationYMatrix[3][3] = {
                {cosValue, 0, sinValue}, {0, 1, 0}, {-sinValue, 0, cosValue}};
            multiMatrix(rotationYMatrix);
        } break;
        case Axis::Z:
        {
            double rotationZMatrix[3][3] = {
                {cosValue, -sinValue, 0}, {sinValue, cosValue, 0}, {0, 0, 1}};
            multiMatrix(rotationZMatrix);
        } break;
        default:
            break;
        }
    }
    /**
     * @brief Translates the reference frame of the point cloud.
     * @param [in] x The amount of translation (in mm) along the X-axis.
     * @param [in] y The amount of translation (in mm) along the Y-axis.
     * @param [in] z The amount of translation (in mm) along the Z-axis.
     */
    void translate(double x, double y, double z)
    {
        translation[0] = x;
        translation[1] = y;
        translation[2] = z;
    }

    /**
     * @brief Check if a custom reference frame has been set using Mech-Eye Viewer
     */
    bool isValid() const
    {
        const double EPSILON = 1e-15;
        double identityRotation[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        double zeroTranslation[3] = {0, 0, 0};
        auto isApprox0 = [EPSILON](double d) { return std::fabs(d) <= EPSILON; };

        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                if (!isApprox0(rotation[row][col] - identityRotation[row][col])) {
                    return true;
                }
            }
        }

        for (int row = 0; row < 3; row++) {
            if (!isApprox0(translation[row] - zeroTranslation[row])) {
                return true;
            }
        }

        return false;
    }

    double rotation[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}; ///< 3*3 rotation matrix.
    double translation[3] = {0, 0, 0}; ///< 3*1 translation vector in [x(mm), y(mm), z(mm)].
private:
    void multiMatrix(const double rotationMatrix[3][3])
    {
        double tempRotation[3][3];
        memcpy(&tempRotation, &rotation, 9 * sizeof(double));

        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                rotation[row][col] = rotationMatrix[row][0] * tempRotation[0][col] +
                                     rotationMatrix[row][1] * tempRotation[1][col] +
                                     rotationMatrix[row][2] * tempRotation[2][col];
            }
        }
    }
};
/**
 * @brief Describes the assignment method of the device IP address.
 */
enum struct IpAssignmentMethod { Unknown, DHCP = 4, Static, LLA };

/**
 * @brief Describes the platform of the camera.
 */
enum struct Platform {
    PLATFORM_A,
    PLATFORM_B,
    PLATFORM_C,
    PLATFORM_D,
    PLATFORM_E,
    PLATFORM_F,
    PLATFORM_G,
    PLATFORM_H
};

inline static std::string ipAssignmentMethodToString(IpAssignmentMethod type)
{
    switch (type) {
    case IpAssignmentMethod::Static:
        return "Static";
    case IpAssignmentMethod::DHCP:
        return "DHCP";
    case IpAssignmentMethod::LLA:
        return "LLA";
    case IpAssignmentMethod::Unknown:
        return "Unknown";
    }
    return "";
}

} // namespace eye

} // namespace mmind
