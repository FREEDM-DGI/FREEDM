////////////////////////////////////////////////////////////////////////////////
/// @file      CPhysicalDeviceManager.hpp
///
/// @author    Stephen Jackson <scj7t4@mst.edu>
///
/// @project   FREEDM DGI
///
/// @description
///     A class to bridge the gap between the DGI and the device interface.
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
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "IPhysicalAdapter.hpp"
#include "types/IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Provides a container that manages physical device instances
class CPhysicalDeviceManager : public boost::noncopyable
{
public:
    /// Type of a pointer to a device manager
    typedef boost::shared_ptr<CPhysicalDeviceManager> ManagerPtr;
    /// A typedef for the mapping of identifier to device ptrs
    typedef std::map<device::Identifier,
                     device::IDevice::DevicePtr> PhysicalDeviceSet;
    /// A typedef providing an iterator for this object
    typedef PhysicalDeviceSet::iterator iterator;
    /// Initialize the physical device manger
    CPhysicalDeviceManager();

    /// Add the specified device to the manager.
    void AddDevice(device::IDevice::DevicePtr resource);

    /// Remove a device by its identifier
    void RemoveDevice(device::Identifier devid);

    /// Devices iterator
    iterator begin() { return m_devices.begin(); };
    iterator end() { return m_devices.end(); };

    /// Get A Device By ID
    device::IDevice::DevicePtr GetDevice(device::Identifier devid);

    /// Tests to see if a device exists
    bool DeviceExists(device::Identifier devid) const;

    /// Gives a count of connected devices
    size_t DeviceCount() const;

    /// Structure of typedefs for GetDevicesOfType
    template <class DeviceType>
    struct PhysicalDevice
    {
        /// Container type returned by the GetDevicesOfType function
        typedef std::list<typename DeviceType::DevicePtr> Container;

        /// Iterator to the container type
        typedef typename Container::iterator iterator;
    };

    /// Selects all the devices of a given type
    template <class DeviceType>
    typename PhysicalDevice<DeviceType>::Container GetDevicesOfType()
    {
        typename PhysicalDevice<DeviceType>::Container result;
        typename DeviceType::DevicePtr next_device;
        iterator it = m_devices.begin();
        iterator end = m_devices.end();

        for( ; it != end; it++ )
        {
            // attempt to convert each managed device to DeviceType
            if( (next_device = device::device_cast<DeviceType>(it->second)) )
            {
                result.push_back(next_device);
            }
        }

        return result;
    }
    
    /// Returns the sum of a key's values for all devices of a type.
    template <class DeviceType>
    SettingValue GetNetValue(std::string key)
    {
        SettingValue result = 0;
        typename DeviceType::DevicePtr next_device;
        iterator it = m_devices.begin();
        iterator end = m_devices.end();

        for( ; it != end; it++ )
        {
            // attempt to convert each managed device to DeviceType
            if( (next_device = device::device_cast<DeviceType>(it->second)) )
            {
                result += next_device->Get(key);
            }
        }

        return result;
    }
    
private:
    /// Mapping From Identifier To Device Set
    PhysicalDeviceSet m_devices;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
