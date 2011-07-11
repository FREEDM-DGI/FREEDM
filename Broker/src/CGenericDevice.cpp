///////////////////////////////////////////////////////////////////////////////
/// @file      CGenericDevice.cpp
///
/// @author    Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
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
///
///////////////////////////////////////////////////////////////////////////////

#include "CGenericDevice.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

namespace freedm {
namespace broker {


///////////////////////////////////////////////////////////////////////////////
/// @fn CGenericDevice
/// @brief A class for describing some very generic device. The device type is
///        always FREEDM_GENERIC
/// @param phymanager The related physical device manager.
/// @param deviceid The identifier for this generic device.
///////////////////////////////////////////////////////////////////////////////
CGenericDevice::CGenericDevice(CPhysicalDeviceManager& phymanager, Identifier deviceid)
            : IPhysicalDevice(phymanager,deviceid,physicaldevices::FREEDM_GENERIC)
{
    // Nothing to see here.
};
///////////////////////////////////////////////////////////////////////////////
/// @fn CGenericDevice::Get
/// @brief Returns the value of some key in the register, or 0.0 if they key is
///        not defined.
/// @param key The key to retrieve from the register.
/// @return The value in the register or 0.0 if the key hasn't been defined.
///////////////////////////////////////////////////////////////////////////////
CGenericDevice::SettingValue CGenericDevice::Get(SettingKey key)
{
    std::map<SettingKey,SettingValue>::iterator ri = m_register.find(key);
    if(ri != m_register.end())
        return ri->second;
    return 0.0;
};
///////////////////////////////////////////////////////////////////////////////
/// @fn CGenericDevice::Set
/// @brief Sets the value of some key to to a new value
/// @param key The key to change.
/// @param value The value to set the key to.
///////////////////////////////////////////////////////////////////////////////
void CGenericDevice::Set(SettingKey key, SettingValue value)
{
    m_register[key] = value;
};

    } // namespace broker
} // namespace freedm
