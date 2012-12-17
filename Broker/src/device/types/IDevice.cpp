////////////////////////////////////////////////////////////////////////////////
/// @file           IDevice.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Physical device interface with variable implementations.
///
/// @functions
///     IDevice::IDevice
///     IDevice::~IDevice
///     IDevice::GetID
///     IDevice::Get
///     IDevice::Set
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

#include "IDevice.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Called by subclass constructors to initialize the device.
///
/// @pre none
/// @post Base of the device created and initialized.
/// @param device The unique device identifier for the device.
/// @param adapter The implementation scheme of the device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::IDevice(const std::string device, IAdapter::Pointer adapter)
    : m_identifier(device)
    , m_adapter(adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Base of the device destroyed.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::~IDevice()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the unique device identifier.
///
/// @pre None.
/// @post Returns the value of m_identifier.
/// @return The unique identifier for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string IDevice::GetID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_identifier;
}

////////////////////////////////////////////////////////////////////////////////
/// Gets a device signal's value from the internal adapter.
///
/// @pre m_adapter must recognize the passed signal.
/// @post m_adapter is queried for the value of the signal.
/// @param signal The signal of the device to retrieve.
/// @return Current value of the specified signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SignalValue IDevice::Get(const std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_adapter->Get(m_identifier, signal);
}

////////////////////////////////////////////////////////////////////////////////
/// Sets a device signal's value in the internal adapter.
///
/// @pre m_adapter must recognize the passed signal.
/// @post m_adapter is queried to set the signal to the passed value.
/// @param signal The signal of the device to update.
/// @param value The value to set for the signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void IDevice::Set(const std::string signal, const SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_adapter->Set(m_identifier, signal, value);
}

} // namespace device
} // namespace broker
} // namespace freedm
