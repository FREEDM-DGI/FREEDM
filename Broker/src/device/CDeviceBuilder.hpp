////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceBuilder.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Handles the construction of new device objects.
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_BUILDER_HPP
#define C_DEVICE_BUILDER_HPP

#include "CDevice.hpp"

#include <map>
#include <set>
#include <string>
#include <utility>

namespace freedm {
namespace broker {
namespace device {

/// Handles construction of all device objects used by the DGI.
////////////////////////////////////////////////////////////////////////////////
/// The device builder class handles the assignment of device type information
/// to new device objects. It stores a map of device types and their associated
/// DeviceInfo structures that is populated when the builder is constructed.
/// Each new device receives a copy of some DeviceInfo stored by the builder
/// which restricts how the device can be used by the DGI.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CDeviceBuilder
{
public:
    /// Default constructor.
    CDeviceBuilder() {}

    /// Constructor that populates the device information using an XML file.
    CDeviceBuilder(std::string filename);

    /// Accessor that returns the device information for a device type.
    DeviceInfo GetDeviceInfo(std::string type);

    /// Builds and returns a shared pointer to a new device object.
    CDevice::Pointer CreateDevice(std::string id, std::string type,
            IAdapter::Pointer adapter);
private:
    /// Stores the variables required to populate the device information map.
    struct BuildVars
    {
        /// Maps a pair of device types to a shared signal name.
        std::map<std::pair<std::string, std::string>, std::string> s_conflict;

        /// Stores the device types that have not been defined.
        std::set<std::string> s_undefined_type;

        /// Stores the device types that have no type information.
        std::set<std::string> s_uninitialized_type;
    };

    /// Recursive function to populate the device information map.
    void ExpandInfo(std::string target, std::set<std::string> path,
            BuildVars & vars);

    /// Map from device type to its associated device information.
    std::map<std::string, DeviceInfo> m_type_to_info;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_BUILDER_HPP
