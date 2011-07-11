// CConnectionManager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "CPhysicalDeviceManager.hpp"
#include "IPhysicalDevice.hpp"

#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

#include <boost/bind.hpp>

namespace freedm {
namespace broker {

///////////////////////////////////////////////////////////////////////////////
/// @fn CPhysicalDeviceManager
/// @brief Constructor for the physical device manager
/// @pre None
/// @post PhysicalDeviceManager is ready to accept & distribute devices.
///////////////////////////////////////////////////////////////////////////////
CPhysicalDeviceManager::CPhysicalDeviceManager()
{
    //intentionally left blank
}
///////////////////////////////////////////////////////////////////////////////
/// @fn CPhysicalDeviceManager::AddDevice
/// @brief Registers a device with the physical device manager.
/// @pre The physical device manager is initialized
/// @post The device has been registered with the manager and is ready to
///       retrieve
/// @param resource a IPhysicalDevice::DevicePtr to the device.
///////////////////////////////////////////////////////////////////////////////
void CPhysicalDeviceManager::AddDevice(IPhysicalDevice::DevicePtr resource)
{
    m_devices[resource->GetID()] = resource;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CPhysicalDeviceManager::RemoveDevice
/// @brief Removes the registration of the device from the manager.
/// @pre The device in question has its identifier in the devices table
/// @post The device with the matching identifier is removed from the table.
/// @param devid The identifier which will be used to specify the device removed
///////////////////////////////////////////////////////////////////////////////
void CPhysicalDeviceManager::RemoveDevice(IPhysicalDevice::Identifier devid)
{
    m_devices.erase(devid);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CPhysicalDeviceManager::GetDevice
/// @brief Returns a shared ptr to the device specified
/// @pre The device in question is in the devices table
/// @post No change
/// @return A DevicePtr to the device, or NULL if the device wasn't found.
///////////////////////////////////////////////////////////////////////////////
IPhysicalDevice::DevicePtr CPhysicalDeviceManager::GetDevice(
    IPhysicalDevice::Identifier devid)
{
    iterator di = m_devices.find(devid);
    if(di != m_devices.end())
    {
        return di->second;
    }
    else
    {
        return IPhysicalDevice::DevicePtr();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CPhysicalDeviceManager::DeviceExists
/// @brief Tests to see if the device exists in the devices table.
/// @pre The object is initialized.
/// @post No change.
/// @param devid The device to look for.
/// @return True if the device is in the device table, false otherwise
///////////////////////////////////////////////////////////////////////////////
bool CPhysicalDeviceManager::DeviceExists(IPhysicalDevice::Identifier devid) const
{
    if(m_devices.count(devid))
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CPhysicalDeviceManager::DeviceCount
/// @brief returns a count of the number of devices being tracked at the moment
/// @pre The object is initialized
/// @post No change.
/// @return The number of devices currently being tracked.
///////////////////////////////////////////////////////////////////////////////
size_t CPhysicalDeviceManager::DeviceCount() const
{
    return m_devices.size();
}


} // namespace broker
} // namespace freedm
