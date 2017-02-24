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
///     CDeviceManager::RevealDevice
///     CDeviceManager::RemoveDevice
///     CDeviceManager::DeviceExists
///     CDeviceManager::GetDevice
///     CDeviceManager::DeviceCount
///     CDeviceManager::GetValues
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
#include "CLogger.hpp"

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
void CDeviceManager::AddDevice(CDevice::Pointer device)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    if( m_devices.count(device->GetID()) > 0 )
    {
        throw std::runtime_error("Duplicate device ID: " + device->GetID());
    }
    if( m_hidden_devices.count(device->GetID()) > 0 )
    {
        throw std::runtime_error("Duplicate device ID: " + device->GetID());
    }

    m_hidden_devices[device->GetID()] = device;
    Logger.Info << "Stored " << device->GetID() << " as hidden device." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Reveals a hidden device.
///
/// @ErrorHandling Throws a std::runtime_error if no such device exists.
/// @pre m_hidden_devices stores the passed identifier.
/// @post Moves a pointer from m_hidden_devices into m_devices.
/// @param devid The identifier of the device pointer to move.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CDeviceManager::RevealDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    if( m_hidden_devices.count(devid) == 0 )
    {
        throw std::runtime_error("Unknown hidden device: " + devid);
    }

    m_devices[devid] = m_hidden_devices[devid];
    m_hidden_devices.erase(devid);

    Logger.Status<< "Revealed the hidden device " << devid << std::endl;
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

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    if( m_devices.erase(devid) != 1 && m_hidden_devices.erase(devid) != 1 )
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
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return( m_devices.count(devid) == 1 );
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a shared pointer to the requested device.
///
/// @ErrorHandling Will output a warning if the device cannot be found.
/// @pre The device must be stored in the device manager.
/// @post Searches m_devices for a device with the passed identifier.
/// @param devid the ID of the device to get.
/// @return A shared pointer to the device, or NULL if it wasn't found.
///
/// @limitations We cannot implement a const version of this function unless
/// we provide a boost::shared_ptr<const DeviceType> typedef for each device
/// class.  The previous implementation of const IDevice::Pointer was incorrect
/// because the syntax made the pointer, not the shared object, constant.
///////////////////////////////////////////////////////////////////////////////
CDevice::Pointer CDeviceManager::GetDevice(std::string devid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    iterator it = m_devices.find(devid);
    if( it != m_devices.end() )
    {
        return it->second;
    }
    else
    {
        Logger.Warn << "Could not get the device " << devid << " from the "
                << " device manager: no such device exists." << std::endl;
        return CDevice::Pointer();
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
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_devices.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a set that contains the stored devices of the given type.
///
/// @pre None.
/// @post Places each device that recognizes the type in the result set.
/// @param type The string identifier for the type of device to retrieve.
/// @return A set that contains the matching subset of m_devices.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
std::set<CDevice::Pointer> CDeviceManager::GetDevicesOfType(std::string type)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    std::set<CDevice::Pointer> result;

    for( iterator it = m_devices.begin(); it != m_devices.end(); it++ )
    {
        if( it->second->HasType(type) )
        {
            result.insert(it->second);
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieves a multiset of values for the specified device signal.
///
/// @pre The signal must be recognized by the specified device.
/// @post Iterates through the devices collecting device signals.
/// @param type The type of the device to collect signals from.
/// @param signal The signal of the device to collect.
/// @return A multiset of values that contains each matching device signal.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
std::multiset<SignalValue> CDeviceManager::GetValues(std::string type,
        std::string signal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::multiset<SignalValue> result;

    std::set<CDevice::Pointer> devices = GetDevicesOfType(type);

    BOOST_FOREACH (CDevice::Pointer device, devices)
    {
        result.insert(device->GetState(signal));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Aggregates a set of device signals using the given binary operation.
///
/// @pre The devices of the specified type must recognize the given signal.
/// @post Performs a binary mathematical operation on a subset of m_devices.
/// @param type The device type that should perform the operation.
/// @param signal The signal of the device to aggregate.
/// @return The aggregate value obtained by applying the binary operation.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
SignalValue CDeviceManager::GetNetValue(std::string type, std::string signal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    SignalValue result = 0;

    std::set<CDevice::Pointer> devices = GetDevicesOfType(type);
    std::set<CDevice::Pointer>::iterator it, end;

    for( it = devices.begin(), end = devices.end(); it != end; it++ )
    {
        result = result + (*it)->GetState(signal);
    }

    return result;
}

} // namespace device
} // namespace broker
} // namespace freedm
