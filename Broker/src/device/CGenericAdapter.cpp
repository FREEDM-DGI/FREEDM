////////////////////////////////////////////////////////////////////////////////
/// @file           CGenericAdapter.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>,
///                 Stephen Jackson <scj7t4@mst.edu>,
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a generic physical device adapter.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "CLogger.hpp"
#include "device/CGenericAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

static CLocalLogger Logger(__FILE__);

////////////////////////////////////////////////////////////////////////////////
/// @function CGenericAdapter::Create()
///
/// @description Creates a new generic device adapter.
///
/// @return a smart pointer to the new device adapter.
////////////////////////////////////////////////////////////////////////////////
CGenericAdapter::AdapterPointer CGenericAdapter::Create()
{
    return AdapterPointer(new CGenericAdapter);
}

////////////////////////////////////////////////////////////////////////////////
/// @function CGenericAdapter::Get(const Identifier, const SettingKey)
///
/// @description Sets the value of a device's setting. If the device is not
///  currently registered with the adapter, it is added. If the setting does not
///  currently exist, it is added with a default value of 0.0
///
/// @param device the unique identifier of the target device.
/// @param key the desired setting on the target device.
///
/// @return the value of the requested setting.
////////////////////////////////////////////////////////////////////////////////
SettingValue CGenericAdapter::Get(const Identifier device,
        const SettingKey key) const
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    
    // Get a map of keys/values from the map of devices/maps.
    // Then look up the value in that map.
    DeviceMap::iterator deviceIter = m_registry.find(device);
    
    if (deviceIter == m_registry.end()) {
        KeyMap km;
        km.insert(std::make_pair(key, 0.0));
        m_registry.insert(std::make_pair(device, km));
        return 0.0;
    }
    
    KeyMap::iterator keyIter = deviceIter->second.find(key);
    if (keyIter == deviceIter->second.end()) {
        m_registry[device].insert(std::make_pair(key, 0.0));
        return 0.0;
    }
    
    return keyIter->second;
}

////////////////////////////////////////////////////////////////////////////////
/// @function CGenericAdapter::Get(const Identifier, const SettingKey)
///
/// @description Sets the value of a device's setting.
///
/// @param device the unique identifier of the target device.
/// @param key the desired setting on the target device.
/// @param value the new value for the setting to take.
////////////////////////////////////////////////////////////////////////////////
void CGenericAdapter::Set(const Identifier device, const SettingKey key,
        const SettingValue value)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    m_registry[device][key] = value;
}

}
}
}
