///////////////////////////////////////////////////////////////////////////////
/// @file       CDeviceGeneric.hpp
///
/// @author     Stephen Jackson <scj7t4@mst.edu>
///             Thomas Roth <tprfh7@mst.edu>
///
/// @compiler   C++
///
/// @project    FREEDM DGI
///
/// @description A generic physical device driver
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missour University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_GENERIC_HPP
#define C_DEVICE_GENERIC_HPP

#include "IPhysicalDevice.hpp"
#include "PhysicalDeviceTypesObsolete.hpp"

namespace freedm {
namespace broker {

/// Very basic generic device with a register to set and get values from.
class CDeviceGeneric : virtual public IPhysicalDevice
{
    public:
        /// Constructor which takes in the manager and device id.
        CDeviceGeneric(CPhysicalDeviceManager& phymanager, Identifier deviceid);

        /// Pulls the setting of some key from the inside.
        SettingValue Get(SettingKey key);

        /// Sets the value of some key to the input value.
        void Set(SettingKey key, SettingValue value);
    private:
        /// The Settings Register
        std::map<SettingKey,SettingValue> m_register;
};

} // namespace broker
} // namespace freedm

#endif // C_DEVICE_GENERIC_HPP
