////////////////////////////////////////////////////////////////////////////////
/// @file           IDevice.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    
///     Physical device interface with variable implementations.
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

#include "device/types/IDevice.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

////////////////////////////////////////////////////////////////////////////////
/// IDevice( Identifier, IPhysicalAdapter::AdapterPtr )
/// @description Called by subclass constructors to initialize the device
/// @pre none
/// @post Base of the device created and initialized
/// @param device The unique device identifier for the device
/// @param adapter The implementation scheme of the device
////////////////////////////////////////////////////////////////////////////////
IDevice::IDevice(Identifier device, IPhysicalAdapter::AdapterPtr adapter )
    : m_identifier(device)
    , m_adapter(adapter)
{
    // skip
}

////////////////////////////////////////////////////////////////////////////////
/// Get( const SettingKey )
/// @description Gets a device setting value from the internal structure
/// @pre m_structure must contain an entry for the passed key
/// @post m_structure is queried for the value of the key
/// @param key The key of the device setting to retrieve
/// @return SettingValue associated with the passed key
////////////////////////////////////////////////////////////////////////////////
SettingValue IDevice::Get( const SettingKey key )
{
    return m_adapter->Get(m_identifier, key);
}

////////////////////////////////////////////////////////////////////////////////
/// Set( const SettingKey, const SettingValue )
/// @description Sets a device setting value in the internal structure
/// @pre m_structure must contain an entry for the passed key
/// @post m_structure is queried to set the key to the passed value
/// @param key The key of the device setting to update
/// @param value The value to set for the setting key
////////////////////////////////////////////////////////////////////////////////
void IDevice::Set( const SettingKey key, const SettingValue value )
{
    m_adapter->Set(m_identifier, key, value);
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
/// @post none
/// @return this device's identifier
////////////////////////////////////////////////////////////////////////////////
Identifier IDevice::GetID() const
{
    return m_identifier;
}

} // namespace device
} // namespace broker
} // namespace freedm
