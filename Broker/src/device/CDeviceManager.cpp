////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceManager.cpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Bridges the gap between the DGI and the device interface.
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

#include "CLogger.hpp"
#include "device/CDeviceManager.hpp"
#include "device/types/CDeviceFid.hpp"

#include <boost/bind.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// Constructor for the physical device manager
///
/// @pre None
/// @post PhysicalDeviceManager is ready to accept & distribute devices.
///////////////////////////////////////////////////////////////////////////////
CDeviceManager::CDeviceManager()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Registers a device with the physical device manager.
///
/// @pre The physical device manager is initialized
/// @post The device has been registered with the manager and is ready to
///  retrieve
///
/// @param resource a IPhysicalDevice::DevicePtr to the device.
///////////////////////////////////////////////////////////////////////////////
void CDeviceManager::AddDevice(IDevice::Pointer resource)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_devices[resource->GetID()] = resource;
}

///////////////////////////////////////////////////////////////////////////////
/// Removes the registration of the device from the manager.
///
/// @pre The device in question has its identifier in the devices table
/// @post The device with the matching identifier is removed from the table.
///
/// @param devid The identifier which will be used to specify the device removed
///////////////////////////////////////////////////////////////////////////////
void CDeviceManager::RemoveDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_devices.erase(devid);
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a shared ptr to the device specified
///
/// @pre The device in question is in the devices table
/// @post No change
///
/// @return A DevicePtr to the device, or NULL if the device wasn't found.
///////////////////////////////////////////////////////////////////////////////
const IDevice::Pointer CDeviceManager::GetDevice(std::string devid)
const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    const_iterator di = m_devices.find(devid);
    if (di != m_devices.end())
    {
        return di->second;
    }
    else
    {
        return IDevice::Pointer();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a shared ptr to the device specified
///
/// @pre The device in question is in the devices table
/// @post No change
///
/// @todo this one might not work, maybe the below one, hi
///
/// @return A DevicePtr to the device, or NULL if the device wasn't found.
///////////////////////////////////////////////////////////////////////////////
IDevice::Pointer CDeviceManager::GetDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return GetDevice(devid);
}

///////////////////////////////////////////////////////////////////////////////
/// Tests to see if the device exists in the devices table.
///
/// @pre None
/// @post No change.
///
/// @param devid The device to look for.
///
/// @return True if the device is in the device table, false otherwise
///////////////////////////////////////////////////////////////////////////////
bool CDeviceManager::DeviceExists(std::string devid) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devices.count(devid);
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a count of the number of devices being tracked at the moment.
///
/// @pre None
/// @post No change
///
/// @return The number of devices currently being tracked.
///////////////////////////////////////////////////////////////////////////////
size_t CDeviceManager::DeviceCount() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devices.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a count of the number of FIDs being tracked at the moment.
///
/// @pre None
/// @post No change
///
/// @return The number of FIDs currently being tracked.
////////////////////////////////////////////////////////////////////////////////
size_t CDeviceManager::CountActiveFids() const
{
    size_t result = 0;
    CDeviceFid::Pointer next_device;

    for (const_iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        // attempt to convert each managed device to DeviceType
        if (( next_device = device_cast<CDeviceFid > ( it->second ) ))
        {
            if( next_device->IsActive() )
            {
                result++;
            }
        }
    }

    return result;
}

/// @todo
std::vector<SignalValue> CDeviceManager::GetValueVector(
    std::string devtype, std::string value)
{
    std::vector<SignalValue> results;

    std::vector<IDevice::Pointer> devices = GetDevicesOfType(devtype);
    std::vector<IDevice::Pointer>::iterator it, end;

    for( it = devices.begin(), end = devices.end(); it != end; it++ )
    {
        results.push_back((*it)->Get(value));
    }

    return results;
}

} // namespace device
} // namespace broker
} // namespace freedm
