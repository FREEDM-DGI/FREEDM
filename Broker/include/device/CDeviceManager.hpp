////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceManager.hpp
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
///     CDeviceManager::GetDevicesOfType
///     CDeviceManager::GetValueVector
///     CDeviceManager::GetValue
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

#ifndef C_DEVICE_MANAGER_HPP
#define C_DEVICE_MANAGER_HPP

#include "IDevice.hpp"
#include "CLogger.hpp"

#include <map>
#include <string>
#include <vector>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger DeviceManagerLogger(__FILE__);
}

class CDeviceManager
    : private boost::noncopyable
{
private:
    /// A typedef for the mapping of identifier to device pointers.
    typedef std::map<std::string, IDevice::Pointer> PhysicalDeviceSet;

public:
    /// A typedef providing an iterator for this object.
    typedef PhysicalDeviceSet::iterator iterator;

    /// A typedef providing a const iterator for this object.
    typedef PhysicalDeviceSet::const_iterator const_iterator;
    
    /// Gets the instance of the device manager.
    static CDeviceManager & Instance();
    
    /// Iterator to the first managed device.
    iterator begin();

    /// Iterator past the last managed device.
    iterator end();

    /// Add the specified device to the manager.
    void AddDevice(IDevice::Pointer device);

    /// Remove a device by its identifier.
    bool RemoveDevice(std::string devid);

    /// Tests to see if a device exists.
    bool DeviceExists(std::string devid) const;
    
    /// Gets a device by its identifier.
    IDevice::Pointer GetDevice(std::string devid);
    
    /// Counts the number of managed devices.
    std::size_t DeviceCount() const;
    
    /// Retrieves all the stored devices of a specified type.
    std::vector<IDevice::Pointer> GetDevicesOfType(std::string type);
    
    /// Retrieves a vector of stored values for the given device signal.
    std::vector<SignalValue> GetValueVector(std::string type,
            std::string signal);
    
    /// Retrieves all the stored devices of a specified type.
    template <class DeviceType>
    std::vector<typename DeviceType::Pointer> GetDevicesOfType();

    /// Retrieves a vector of stored values from the given device class.
    template <class DeviceType>
    std::vector<SignalValue> GetValueVector(
            SignalValue (DeviceType::*getter)() const) const;
    
    /// Returns the result of a binary operation on a set of device signals.
    template <class BinaryOp>
    SignalValue GetValue(std::string type, std::string signal, BinaryOp math);
    
    /// Returns the result of a binary operation on a set of device signals.
    template <class DeviceType, class BinaryOp>
    SignalValue GetValue(SignalValue (DeviceType::*getter)() const,
            BinaryOp math) const;

private:
    /// Private constructor.
    CDeviceManager();
    
    /// Mapping from identifiers to device pointers.
    PhysicalDeviceSet m_devices;
};

///////////////////////////////////////////////////////////////////////////////
/// Creates a vector that contains the devices of the templated type.
///
/// @pre DeviceType must have a typedef for DeviceType::Pointer.
/// @post Constructs the vector through dynamic pointer casts.
/// @return A vector that contains the matching subset of m_devices.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class DeviceType>
std::vector<typename DeviceType::Pointer> CDeviceManager::GetDevicesOfType()
{
    DeviceManagerLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::vector<typename DeviceType::Pointer> result;
    typename DeviceType::Pointer next_device;

    for( iterator it = m_devices.begin(); it != m_devices.end(); it++ )
    {
        // attempt to convert each managed device to DeviceType
        if( (next_device = device_cast<DeviceType>(it->second)) )
        {
            result.push_back(next_device);
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a vector that contains values from devices of the templated type.
///
/// @pre DeviceType must hav ea typedef for IDevice::Pointer.
/// @post Calls the function pointer on each device of templated type.
/// @param getter The function to call on each device of the matching type.
/// @return A vector that contains the results of the function pointer calls.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class DeviceType>
std::vector<SignalValue> CDeviceManager::GetValueVector(
        SignalValue(DeviceType::*getter)( ) const) const
{
    DeviceManagerLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::vector<SignalValue> result;
    typename DeviceType::Pointer next_device;

    for( const_iterator it = m_devices.begin(); it != m_devices.end(); it++ )
    {
        // attempt to convert each managed device to DeviceType
        if( (next_device = device_cast<DeviceType>(it->second)) )
        {
            result.push_back(( (*next_device).*(getter) )( ));
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Aggregates a set of device signals using the given binary operation.
///
/// @pre DeviceType must hav ea typedef for IDevice::Pointer.
/// @pre The devices of the specified type must recognize the given signal.
/// @post Performs a binary mathematical operation on a subset of m_devices.
/// @param type The device type that should perform the operation.
/// @param signal The signal of the device to aggregate.
/// @param math The operation to perform on the device signal.
/// @return The aggregate value obtained by applying the binary operation.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class BinaryOp>
SignalValue CDeviceManager::GetValue(std::string type, std::string signal,
        BinaryOp math)
{
    DeviceManagerLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    SignalValue result = 0;

    std::vector<IDevice::Pointer> devices = GetDevicesOfType(type);
    std::vector<IDevice::Pointer>::iterator it, end;

    for( it = devices.begin(), end = devices.end(); it != end; it++ )
    {
        result = math(result, (*it)->Get(signal));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Aggregates a set of device signals using the given binary operation.
///
/// @pre DeviceType must hav ea typedef for IDevice::Pointer.
/// @post Performs a binary mathematical operation on a subset of m_devices.
/// @param getter The function to call on each device to get the signal.
/// @param math The operation to perform on the device signal.
/// @return The aggregate value obtained by applying the binary operation.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
template <class DeviceType, class BinaryOp>
SignalValue CDeviceManager::GetValue(SignalValue(DeviceType::*getter)( ) const,
        BinaryOp math) const
{
    DeviceManagerLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    SignalValue result = 0;
    typename DeviceType::Pointer next_device;

    for( const_iterator it = m_devices.begin(); it != m_devices.end(); it++ )
    {
        // attempt to convert each managed device to DeviceType
        if( (next_device = device_cast<DeviceType>(it->second)) )
        {
            result = math(result, (( *next_device ).*( getter ))( ));
        }
    }

    return result;
}

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_MANAGER_HPP
