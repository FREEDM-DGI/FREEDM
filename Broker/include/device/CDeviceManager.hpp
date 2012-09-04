////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceManager.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Bridges the gap between the DGI and the device interface.
///
/// @functions
///     CDeviceManager::GetDevicesOfType
///     CDeviceManager::GetValue
///     CDeviceManager::GetValueVector
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

#ifndef PHYSICALDEVICEMANAGER_HPP
#define PHYSICALDEVICEMANAGER_HPP

#include "IAdapter.hpp"
#include "types/IDevice.hpp"

#include <list>
#include <map>
#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

/// @todo This class is not sufficiently commented. Comment it better.
/// @todo Need to use typedefs for the function pointer types to be legible.
/// Provides a container that manages physical device instances
class CDeviceManager : private boost::noncopyable
{
private:
    /// A typedef for the mapping of identifier to device ptrs
    typedef std::map<std::string, IDevice::Pointer> PhysicalDeviceSet;

public:
    /// Type of a pointer to a device manager
    typedef boost::shared_ptr<CDeviceManager> Pointer;

    /// A typedef providing an iterator for this object
    typedef PhysicalDeviceSet::iterator iterator;

    /// A typedef providing a const iterator for this object
    typedef PhysicalDeviceSet::const_iterator const_iterator;

    /// Initialize the physical device manger
    CDeviceManager();

    /// Add the specified device to the manager.
    void AddDevice(IDevice::Pointer resource);

    /// Remove a device by its identifier
    void RemoveDevice(std::string devid);

    /// Gets a device by its identifier
    IDevice::Pointer GetDevice(std::string devid);

    /// Gets a device by its identifier
    const IDevice::Pointer GetDevice(std::string devid) const;

    /// Tests to see if a device exists
    bool DeviceExists(std::string devid) const;

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

    /// Const iterator to the first managed device.
    const_iterator begin() const
    {
        return m_devices.begin();
    };

    /// Const iterator past the last managed device.
    const_iterator end() const
    {
        return m_devices.end();
    };

    /// Retrieves all registered devices of a particular type.
    template <class DeviceType>
    const std::vector<typename DeviceType::Pointer> GetDevicesOfType();

    /// @todo takes a bit of effort to make this const
    template <class BinaryOp>
    SignalValue GetValue(std::string devtype,
                          std::string value,
                          BinaryOp math);

    /// @todo takes a bit of effort to make this const
    std::vector<SignalValue> GetValueVector(std::string devtype,
        std::string value);

    /// @todo
    template <class DeviceType, class BinaryOp>
    SignalValue GetValue(SignalValue(DeviceType::*getter)( ) const,
    BinaryOp math) const;

    /// @todo
    template <class DeviceType>
    std::vector<SignalValue> GetValueVector(
        SignalValue(DeviceType::*getter)( ) const) const;
 
    /// Gives a count of connected FIDs.
    size_t CountActiveFids() const;

    /// Retrieves all the stored devices of a specified type.
    std::vector<IDevice::Pointer> GetDevicesOfType(std::string type);

private:
    /// Mapping from device identifiers to device set
    PhysicalDeviceSet m_devices;
};

/// Selects all the devices of a given type
template <class DeviceType>
const std::vector<typename DeviceType::Pointer>
CDeviceManager::GetDevicesOfType()
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
template <class DeviceType, class BinaryOp>
SignalValue CDeviceManager::GetValue(
        SignalValue(DeviceType::*getter)( ) const,
        BinaryOp math) const
{
    SignalValue result = 0;
    typename DeviceType::Pointer next_device;

    for (const_iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        // attempt to convert each managed device to DeviceType
        if (( next_device = device_cast<DeviceType > ( it->second ) ))
        {
            result = math(result, ( ( *next_device ).*( getter ) )( ) );
        }
    }

    return result;
}

/// @todo
template <class DeviceType>
std::vector<SignalValue> CDeviceManager::GetValueVector(
SignalValue(DeviceType::*getter)( ) const) const
{
    std::vector<SignalValue> results;
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

/// @todo
template <class BinaryOp>
SignalValue CDeviceManager::GetValue(std::string devtype,
std::string value, BinaryOp math)
{
    SignalValue result = 0;

    std::vector<IDevice::Pointer> devices = GetDevicesOfType(devtype);
    std::vector<IDevice::Pointer>::iterator it, end;

    for( it = devices.begin(), end = devices.end(); it != end; it++ )
    {
        result = math(result, (*it)->Get(value));
    }

    return result;
}

} // namespace device
} // namespace broker
} // namespace freedm

#endif // PHYSICALDEVICEMANAGER_HPP
