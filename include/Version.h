#pragma once
#include <string>
#include <regex>
#include "api_global.h"

namespace mmind {

namespace eye {

/**
 * @brief Gets the version of Mech-Eye API.
 */
MMIND_API_EXPORT std::string getApiVersionInfo();

/**
 * @brief Describes the version information.
 */
class Version
{
public:
    /**
     * @brief Default constructor.
     */
    Version() = default;

    /**
     * @brief Constructor.
     */
    Version(int major, int minor, int patch) : _major(major), _minor(minor), _patch(patch) {}

    /**
     * @brief Constructor.
     */
    explicit Version(const std::string& version) { fromString(version); }

    /**
     * @brief Overloads the == operator to determine if two Version objects are equal.
     */
    bool operator==(const Version& other) const { return toString() == other.toString(); }

    /**
     * @brief Overloads the != operator to determine if two Version objects are unequal.
     */
    bool operator!=(const Version& other) const { return !(*this == (other)); }

    /**
     * @brief Overloads the >= operator to determine if one Version object is greater than or equal
     * to the other.
     */
    bool operator>=(const Version& other) const
    {
        return _major > other._major ||
               (_major == other._major &&
                (_minor > other._minor || (_minor == other._minor && _patch >= other._patch)));
    }

    /**
     * @brief Overloads the < operator to determine if one Version object is smaller than the other.
     */
    bool operator<(const Version& other) const { return !(*this >= (other)); }

    /**
     * @brief Overloads the <= operator to determine if one Version object is smaller than or equal
     * to the other.
     */
    bool operator<=(const Version& other) const { return *this < other || *this == other; }

    /**
     * @brief Converts a Version object to a string.
     */
    std::string toString() const
    {
        char buff[16] = {0};
        snprintf(buff, sizeof(buff), "%d.%d.%d", _major, _minor, _patch);
        return buff;
    }

    /**
     * @brief Converts a version in the string format to a Version object.
     */
    void fromString(const std::string& version)
    {
        std::regex rege(R"((\d+).(\d+).(\d+))");
        std::smatch result;
        if (std::regex_search(version, result, rege)) {
            _major = std::stoi(result.str(1));
            _minor = std::stoi(result.str(2));
            _patch = std::stoi(result.str(3));
        }
    }

    /**
     * @brief Checks if a Version object is empty.
     */
    bool isEmpty() const { return *this == Version(); }

private:
    int _major = 0;
    int _minor = 0;
    int _patch = 0;
};
} // namespace eye

} // namespace mmind
