////////////////////////////////////////////////////////////////////////////////
/// @file           IDevice.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Physical device interface with variable implementations.
///
/// @license
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "IDevice.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

////////////////////////////////////////////////////////////////////////////////
/// IDevice( CPhysicalDeviceManager &, Identifier, IDeviceStructure::DevicePtr )
/// @description Called by subclass constructors to initialize the device
/// @pre none
/// @post Base of the device created and initialized
/// @param manager The device manager that will handle the device
/// @param device The unique device identifier for the device
/// @param structure The implementation scheme of the device
////////////////////////////////////////////////////////////////////////////////
IDevice::IDevice( CPhysicalDeviceManager & manager, Identifier device,
                  IDeviceStructure::DevicePtr structure )
    : m_manager(manager)
    , m_device(device)
    , m_structure(structure)
{
    // skip
}

////////////////////////////////////////////////////////////////////////////////
/// Get( const SettingKey & )
/// @description Gets a device setting value from the internal structure
/// @pre m_structure must contain an entry for the passed key
/// @post m_structure is queried for the value of the key
/// @param key The key of the device setting to retrieve
/// @return SettingValue associated with the passed key
////////////////////////////////////////////////////////////////////////////////
SettingValue IDevice::Get( const SettingKey & key )
{
    return m_structure->Get(key);
}

////////////////////////////////////////////////////////////////////////////////
/// Set( const SettingKey &, const SettingValue & )
/// @description Sets a device setting value in the internal structure
/// @pre m_structure must contain an entry for the passed key
/// @post m_structure is queried to set the key to the passed value
/// @param key The key of the device setting to update
/// @param value The value to set for the setting key
////////////////////////////////////////////////////////////////////////////////
void IDevice::Set( const SettingKey & key, const SettingValue & value )
{
    m_structure->Set(key,value);
}

////////////////////////////////////////////////////////////////////////////////
/// turnOn()
/// @description Turns the device on
////////////////////////////////////////////////////////////////////////////////
void IDevice::turnOn()
{
    m_structure->Set("onOffSwitch",1);
}

////////////////////////////////////////////////////////////////////////////////
/// turnOff()
/// @description Turns the device off
////////////////////////////////////////////////////////////////////////////////
void IDevice::turnOff()
{
    m_structure->Set("onOffSwitch",0);
}

////////////////////////////////////////////////////////////////////////////////
/// GetManager()
/// @description Accessor for m_manager
/// @pre none
/// @post returns a reference to the device manager
/// @return reference to m_manager
////////////////////////////////////////////////////////////////////////////////
CPhysicalDeviceManager & IDevice::GetManager()
{
    return m_manager;
}

////////////////////////////////////////////////////////////////////////////////
/// GetManager() const
/// @description Accessor for m_manager
/// @pre none
/// @post returns a const reference to the device manager
/// @return const reference to m_manager
////////////////////////////////////////////////////////////////////////////////
const CPhysicalDeviceManager & IDevice::GetManager() const
{
    return m_manager;
}

////////////////////////////////////////////////////////////////////////////////
/// Lock()
/// @description Obtains a lock over the device mutex
/// @pre none
/// @post blocks until m_mutex is locked by the calling thread
////////////////////////////////////////////////////////////////////////////////
void IDevice::Lock()
{
    m_mutex.lock();
}

////////////////////////////////////////////////////////////////////////////////
/// Unlock()
/// @description Releases an obtained lock over the device mutex
/// @pre calling thread must have the mutex locked
/// @post releases the lock on m_mutex
////////////////////////////////////////////////////////////////////////////////
void IDevice::Unlock()
{
    m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////
/// TryLock()
/// @description Tries to obtain a lock over the device mutex
/// @pre none
/// @post locks m_mutex if the function returns true
/// @return true if m_mutex lock acquired, false otherwise
////////////////////////////////////////////////////////////////////////////////
bool IDevice::TryLock()
{
    return m_mutex.try_lock();
}

////////////////////////////////////////////////////////////////////////////////
/// GetID() const
/// @description Accessor for the unique device identifier
/// @pre none
/// @post Returns const reference to m_device
/// @return const reference to m_device
////////////////////////////////////////////////////////////////////////////////
const Identifier & IDevice::GetID() const
{
    return m_device;
}

} // namespace device
} // namespace broker
} // namespace freedm
