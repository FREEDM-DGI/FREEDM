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
///     IDevice::GetStateSet
///     IDevice::GetCommandSet
///     IDevice::HasStateSignal
///     IDevice::HasCommandSignal
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

#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor for derived classes.
///
/// @pre None.
/// @post Constructs the base device class.
/// @param identifier The unique identifier for the device.
/// @param adapter The implementation scheme of the device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::IDevice(const std::string identifier, IAdapter::Pointer adapter)
    : m_identifier(identifier)
    , m_adapter(adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destroys the base device class.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
virtual IDevice::~IDevice()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the device identifier.
///
/// @pre None.
/// @post Returns m_identifier.
/// @return The unique device identifier.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string IDevice::GetID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_identifier;
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the value of a device signal from the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if IDevice::HasStateSignal
/// returns false for the passed signal.
/// @pre m_adapter must recognized the passed signal.
/// @post m_adapter is queried for the value of the passed signal.
/// @param signal The signal to retrieve from m_adapter.
/// @return The current value of the specified signal.
///
/// @limitations This function can fail even if IDevice::HasStateSignal returns
/// true for the passed signal.  The adapter, not the device, must recognize
/// the given state signal.
////////////////////////////////////////////////////////////////////////////////
SignalValue IDevice::Get(const std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( !HasStateSignal(signal) )
    {
        throw std::runtime_error("The device, " + m_identifier
                + ", does not recognize the state signal: " + signal);
    }
    
    return m_adapter->Get(m_identifier, signal);
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the value of a device signal in the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if IDevice::HasCommandSignal
/// returns false for the passed signal.
/// @pre m_adapter must recognized the passed signal.
/// @post m_adapter is queried to set the value of the passed signal.
/// @param signal The signal to update in m_adapter.
/// @param value The value to set for the signal.
///
/// @limitations This function can fail even if IDevice::HasCommandSignal
/// returns true for the passed signal.  The adapter, not the device, must
/// recognize the given command signal.
////////////////////////////////////////////////////////////////////////////////
void IDevice::Set(const std::string signal, const SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( !HasCommandSignal(signal) )
    {
        throw std::runtime_error("The device, " + m_identifier
                + ", does not recognize the command signal: " + signal);
    }
    
    m_adapter->Set(m_identifier, signal, value);
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the set of state signals.
///
/// @pre None.
/// @post Returns m_StateSet.
/// @return The set of state signals.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::set<std::string> IDevice::GetStateSet() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_StateSet;
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the set of command signals.
///
/// @pre None.
/// @post Returns m_CommandSet.
/// @return The set of command signals.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::set<std::string> IDevice::GetCommandSet() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_CommandSet;
}

////////////////////////////////////////////////////////////////////////////////
/// Checks if the device recognizes a string as a state signal.
///
/// @pre None.
/// @post Searches m_StateSet for the passed signal.
/// @param signal The string identifier of the state signal.
/// @return True if m_StateSet contains signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
bool IDevice::HasStateSignal(const std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_StateSet.count(signal) > 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Checks if the device recognizes a string as a command signal.
///
/// @pre None.
/// @post Searches m_CommandSet for the passed signal.
/// @param signal The string identifier of the command signal.
/// @return True if m_CommandSet contains signal.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
bool IDevice::HasCommandSignal(const std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_CommandSet.count(signal) > 0;
}

} // namespace device
} // namespace broker
} // namespace freedm
