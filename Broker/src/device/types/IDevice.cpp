////////////////////////////////////////////////////////////////////////////////
/// @file           IDevice.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Physical device interface with variable implementations.
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
#include "device/types/IDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::IDevice(Identifier, IPhysicalAdapter::AdapterPtr)
///
/// @description Called by subclass constructors to initialize the device
/// @pre none
/// @post Base of the device created and initialized
/// @param device The unique device identifier for the device
/// @param adapter The implementation scheme of the device
////////////////////////////////////////////////////////////////////////////////
IDevice::IDevice(Identifier device, IPhysicalAdapter::Pointer adapter)
: m_identifier(device), m_adapter(adapter), m_mutex()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::~IDevice()
///
/// @description Virtual destructor for derived classes.
////////////////////////////////////////////////////////////////////////////////
IDevice::~IDevice()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::Get(const SettingKey) const
/// @description Gets a device setting value from the internal structure
/// @pre m_structure must contain an entry for the passed key
/// @post m_structure is queried for the value of the key
/// @param key The key of the device setting to retrieve
/// @return SettingValue associated with the passed key
////////////////////////////////////////////////////////////////////////////////
SettingValue IDevice::Get(const SettingKey key) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_adapter->Get(m_identifier, key);
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::Set(const SettingKey, const SettingValue)
/// @description Sets a device setting value in the internal structure
/// @pre m_structure must contain an entry for the passed key
/// @post m_structure is queried to set the key to the passed value
/// @param key The key of the device setting to update
/// @param value The value to set for the setting key
////////////////////////////////////////////////////////////////////////////////
void IDevice::Set(const SettingKey key, const SettingValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_adapter->Set(m_identifier, key, value);
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::Lock()
/// @description Obtains a lock over the device mutex
/// @pre none
/// @post blocks until m_mutex is locked by the calling thread
////////////////////////////////////////////////////////////////////////////////
void IDevice::Lock()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_mutex.lock();
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::Unlock()
/// @description Releases an obtained lock over the device mutex
/// @pre calling thread must have the mutex locked
/// @post releases the lock on m_mutex
////////////////////////////////////////////////////////////////////////////////
void IDevice::Unlock()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::TryLock()
/// @description Tries to obtain a lock over the device mutex
/// @pre none
/// @post locks m_mutex if the function returns true
/// @return true if m_mutex lock acquired, false otherwise
////////////////////////////////////////////////////////////////////////////////
bool IDevice::TryLock()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_mutex.try_lock();
}

////////////////////////////////////////////////////////////////////////////////
/// IDevice::GetID() const
/// @description Accessor for the unique device identifier
/// @pre none
/// @post none
/// @return this device's identifier
////////////////////////////////////////////////////////////////////////////////
Identifier IDevice::GetID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_identifier;
}

} // namespace device
} // namespace broker
} // namespace freedm
