////////////////////////////////////////////////////////////////////////////////
/// @file       CPhysicalDeviceManager.hpp
///
/// @author     Stephen Jackson <scj7t4@mst.edu>
/// @author     Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Bridges the gap between the DGI and the device interface.
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

#ifndef PHYSICALDEVICEMANAGER_HPP
#define PHYSICALDEVICEMANAGER_HPP

#include <string>
#include <map>
#include <list>

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "IPhysicalAdapter.hpp"
#include "types/IDevice.hpp"

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

namespace freedm {
namespace broker {
namespace device {

/// Provides a container that manages physical device instances
class CPhysicalDeviceManager : private boost::noncopyable
{
public:
    /// Type of a pointer to a device manager
    typedef boost::shared_ptr<CPhysicalDeviceManager> Pointer;

    /// A typedef for the mapping of identifier to device ptrs
    typedef std::map<Identifier, IDevice::Pointer> PhysicalDeviceSet;

    /// A typedef providing an iterator for this object
    typedef PhysicalDeviceSet::iterator iterator;

    /// A typedef providing a const iterator for this object
    typedef PhysicalDeviceSet::const_iterator const_iterator;

    /// Initialize the physical device manger
    CPhysicalDeviceManager();

    /// Add the specified device to the manager.
    void AddDevice(IDevice::Pointer resource);

    /// Remove a device by its identifier
    void RemoveDevice(Identifier devid);

    /// Gets a device by its identifier
    IDevice::Pointer GetDevice(Identifier devid);

    /// Gets a device by its identifier
    const IDevice::Pointer GetDevice(Identifier devid) const;

    /// Tests to see if a device exists
    bool DeviceExists(Identifier devid) const;

    /// Gives a count of connected devices
    size_t DeviceCount() const;

    /// Iterator to the first managed device.
    iterator begin()
    {
        return m_devices.begin();
    };

    /// Iterator past the last managed device.
    iterator end()
    {
        return m_devices.end();
    };

    /// Iterator to the first managed device.
    const_iterator begin() const
    {
        return m_devices.begin();
    };

    /// Iterator past the last managed device.
    const_iterator end() const
    {
        return m_devices.end();
    };

    /// @todo
    template <class DeviceType>
    const std::vector<typename DeviceType::Pointer> GetDevicesOfType();

    /// @todo
    SettingValue GetValue(std::string devtype, std::string value
        SettingValue(*math)( SettingValue, SettingValue )) const;
    
    /// @todo
    std::vector<SettingValue> GetValueVector(std::string devtype,
        std::string value) const;
    
    /// @todo
    template <class DeviceType>
    SettingValue GetValue(SettingValue(DeviceType::*getter)( ) const,
        SettingValue(*math)( SettingValue, SettingValue )) const;

    /// @todo
    template <class DeviceType>
    std::vector<SettingValue> GetValueVector(
        SettingValue(DeviceType::*getter)( ) const) const;
 
    /// @todo
    unsigned int CountActiveFids() const;

    /// Retrieves all the stored devices of a specified type.
    const std::vector<IDevice::Pointer> GetDevicesOfType(std::string type);
private:
    /// Mapping From Identifier To Device Set
    PhysicalDeviceSet m_devices;
};

/// Selects all the devices of a given type
template <class DeviceType>
const std::vector<typename DeviceType::Pointer>
CPhysicalDeviceManager::GetDevicesOfType()
{
    std::vector<typename DeviceType::Pointer> result;
    typename DeviceType::Pointer next_device;

    for (iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        // attempt to convert each managed device to DeviceType
        if (( next_device = device_cast<DeviceType > ( it->second ) ))
        {
            result.push_back(next_device);
        }
    }

    return result;
}

// @todo
template <class DeviceType>
SettingValue CPhysicalDeviceManager::GetValue(
SettingValue(DeviceType::*getter)( ) const,
SettingValue(*math)( SettingValue, SettingValue )) const
{
    SettingValue result;
    typename DeviceType::Pointer next_device;

    for (const_iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        // attempt to convert each managed device to DeviceType
        if (( next_device = device_cast<DeviceType > ( it->second ) ))
        {
            result = math(result, ( ( ( *next_device ).*( getter ) )( ) ));
        }
    }

    return result;
}

/// @todo
template <class DeviceType>
std::vector<SettingValue> CPhysicalDeviceManager::GetValueVector(
SettingValue(DeviceType::*getter)( ) const) const
{
    std::vector<SettingValue> results;
    typename DeviceType::Pointer next_device;

    for (const_iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        // attempt to convert each managed device to DeviceType
        if (( next_device = device_cast<DeviceType > ( it->second ) ))
        {
            results.push_back(( ( *next_device ).*( getter ) )( ));
        }
    }

    return results;
}

} // namespace device
} // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
