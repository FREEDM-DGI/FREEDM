////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceManager.cpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Bridges the gap between the DGI and the device interface.
///
/// @functions
///     CDeviceManager::Instance
///     CDeviceManager::CDeviceManager
///     CDeviceManager::begin
///     CDeviceManager::end
///     CDeviceManager::AddDevice
///     CDeviceManager::RemoveDevice
///     CDeviceManager::DeviceExists
///     CDeviceManager::GetDevice
///     CDeviceManager::DeviceCount
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

#include "CDeviceManager.hpp"

#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieves the singleton device manager instance.
///
/// @pre None.
/// @post Creates a new device manager on the first call.
/// @return The global instance of CDeviceManager.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CDeviceManager & CDeviceManager::Instance()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    static CDeviceManager instance;
    return instance;
}

///////////////////////////////////////////////////////////////////////////////
/// Constructor for the device manager
///
/// @pre None
/// @post DeviceManager is ready to accept & distribute devices.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CDeviceManager::CDeviceManager()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets an iterator to the beginning of the managed devices.
///
/// @pre None.
/// @post Returns an iterator to a managed device.
/// @return An iterator to the first managed device.
///
/// @limitations The iterator will be null if no devices exist.
///////////////////////////////////////////////////////////////////////////////
CDeviceManager::iterator CDeviceManager::begin()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devices.begin();
}

///////////////////////////////////////////////////////////////////////////////
/// Gets an iterator past the end of the managed devices.
///
/// @pre None.
/// @post Returns an iterator past the end of the managed devices.
/// @return An iterator past the last managed device.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CDeviceManager::iterator CDeviceManager::end()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devices.end();
}

///////////////////////////////////////////////////////////////////////////////
/// Registers a device with the physical device manager.
///
/// @ErrorHandling Throws a std::runtime_error of another device has been
/// registered with the same device identifier.
/// @SharedMemory Stores a shared pointer to the given device.
/// @pre There must not be a device registered with the same identifier.
/// @post The device is stored in the device set m_devices.
/// @param device The device pointer to store in the manager.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CDeviceManager::AddDevice(IDevice::Pointer device)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_devices.count(device->GetID()) > 0 )
    {
        throw std::runtime_error("The device " + device->GetID()
                + " is already registered with the device manager.");
    }
    
    m_devices[device->GetID()] = device;
    Logger.Info << "Stored the device " << device->GetID() << " in the device "
            << "manager." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Removes a device from the manager.
///
/// @ErrorHandling Will output a warning if the device cannot be found.
/// @pre None.
/// @post The device with the matching identifier is removed from m_devices.
/// @param devid The identifier of the device to remove.
/// @return True if a device has been removed, false otherwise.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
bool CDeviceManager::RemoveDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_devices.erase(devid) != 1 )
    {
        Logger.Warn << "Could not remove the device " << devid << " from the "
                << " device manager: no such device exists." << std::endl;
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// Tests to see if the device exists in the devices manager.
///
/// @pre None
/// @post Searches m_device for the device.
/// @param devid The identifier of the device to find.
/// @return True if the device is in the device manager, false otherwise
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
bool CDeviceManager::DeviceExists(std::string devid) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return( m_devices.count(devid) == 1 );
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a shared pointer to the requested device.
///
/// @ErrorHandling Will output a warning if the device cannot be found.
/// @pre The device must be stored in the device manager.
/// @post Searches m_devices for a device with the passed identifier.
/// @return A shared pointer to the device, or NULL if it wasn't found.
///
/// @limitations We cannot implement a const version of this function unless
/// we provide a boost::shared_ptr<const DeviceType> typedef for each device
/// class.  The previous implementation of const IDevice::Pointer was incorrect
/// because the syntax made the pointer, not the shared object, constant.
///////////////////////////////////////////////////////////////////////////////
IDevice::Pointer CDeviceManager::GetDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    iterator it = m_devices.find(devid);
    if( it != m_devices.end() )
    {
        return it->second;
    }
    else
    {
        Logger.Warn << "Could not get the device " << devid << " from the "
                << " device manager: no such device exists." << std::endl;
        return IDevice::Pointer();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a count of the number of devices stored by the device manager.
///
/// @pre None
/// @post Returns the size of m_devices.
/// @return The number of devices currently stored.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
std::size_t CDeviceManager::DeviceCount() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devices.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieves a vector of values for the specified device signal.
///
/// @pre The signal must be recognized by the specified device.
/// @post Iterates through the devices collecting device signals.
/// @param type The type of the device to collect signals from.
/// @param signal The signal of the device to collect.
/// @return A vector of values that contains each matching device signal.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
std::vector<SignalValue> CDeviceManager::GetValueVector(std::string type,
        std::string signal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::vector<SignalValue> result;

    std::vector<IDevice::Pointer> devices = GetDevicesOfType(type);

    BOOST_FOREACH (IDevice::Pointer device, devices)
    {
        result.push_back(device->Get(signal));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Aggregates a set of device signals using the given binary operation.
///
/// @pre DeviceType must have a typedef for IDevice::Pointer.
/// @pre The devices of the specified type must recognize the given signal.
/// @post Performs a binary mathematical operation on a subset of m_devices.
/// @param type The device type that should perform the operation.
/// @param signal The signal of the device to aggregate.
/// @param math The operation to perform on the device signal.
/// @return The aggregate value obtained by applying the binary operation.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
SignalValue CDeviceManager::GetNetValue(std::string type, std::string signal)
{
    DeviceManagerLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    SignalValue result = 0;

    std::vector<IDevice::Pointer> devices = GetDevicesOfType(type);
    std::vector<IDevice::Pointer>::iterator it, end;

    for( it = devices.begin(), end = devices.end(); it != end; it++ )
    {
        result = result + (*it)->Get(signal);
    }

    return result;
}

} // namespace device
} // namespace broker
} // namespace freedm
