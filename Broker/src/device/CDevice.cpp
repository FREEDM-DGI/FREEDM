////////////////////////////////////////////////////////////////////////////////
/// @file         CDevice.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Defines the interface for physical devices.
///
/// @functions
///     CDevice::CDevice
///     CDevice::GetID
///     CDevice::HasType
///     CDevice::HasState
///     CDevice::HasCommand
///     CDevice::GetState
///     CDevice::GetStateSet
///     CDevice::GetCommandSet
///     CDevice::SetCommand
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

#include "CDevice.hpp"
#include "CLogger.hpp"

#include <ostream>
#include <stdexcept>

#include <boost/foreach.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
CLocalLogger Logger(__FILE__);
}


////////////////////////////////////////////////////////////////////////////////
/// Outputs the device information structure to the passed stream.
///
/// @pre None.
/// @post Outputs each member variable of info on a separate line.
/// @param os The stream that the device information should output to.
/// @param info The device information to output to the stream.
/// @return A reference to the passed output stream.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::ostream & operator<<(std::ostream & os, const DeviceInfo & info)
{
    os << "Types:";
    BOOST_FOREACH(std::string type, info.s_type)
    {
        os << " " << type;
    }

    os << "\nStates:";
    BOOST_FOREACH(std::string state, info.s_state)
    {
        os << " " << state;
    }

    os << "\nCommands:";
    BOOST_FOREACH(std::string command, info.s_command)
    {
        os << " " << command;
    }

    return os;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor for device objects.
///
/// @pre The adapter must be configured to store the new device object.
/// @post Constructs a device object.
/// @param id The unique identifier for this device object.
/// @param info The structure that defines the type of this device object.
/// @param adapter The adapter that handles storage for this device object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDevice::CDevice(std::string id, DeviceInfo info, IAdapter::Pointer adapter)
    : m_devid(id)
    , m_devinfo(info)
    , m_adapter(adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Info << "CREATED NEW DEVICE:\n" << m_devid << "\n" << m_devinfo
            << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the unique device identifier.
///
/// @pre None.
/// @post Returns m_devid.
/// @return The unique device identifier.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string CDevice::GetID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devid;
}

////////////////////////////////////////////////////////////////////////////////
/// Checks if the device can be used as a specific type.
///
/// @pre None.
/// @post Returns true if m_devinfo.s_type contains the passed value.
/// @param type The string identifier of some device type.
/// @return True if this device can be used as the specified type.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
bool CDevice::HasType(std::string type) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_type.count(type) > 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Checks if the device has some specified state.
///
/// @pre None.
/// @post Returns true if m_devinfo.s_state contains the passed value.
/// @param signal The string identifier of some state signal.
/// @return True if this device has the specified state.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
bool CDevice::HasState(std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_state.count(signal) > 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Checks if the device has some specified command.
///
/// @pre None.
/// @post Returns true if m_devinfo.s_command contains the passed value.
/// @param signal The string identifier of some command signal.
/// @return True if this device has the specified command.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
bool CDevice::HasCommand(std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_command.count(signal) > 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the value of a device state from the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if CDevice::HasState returns
/// false for the passed string identifier.
/// @pre The device must recognize the passed signal.
/// @post m_adapter is queried for the value of the passed signal.
/// @param signal The string identifier for the state signal to retrieve.
/// @return The current value of the specified state signal.
///
/// @limitations This function can still fail even if CDevice::HasState returns
/// true. This case happens when m_adapter is not configured to store all the
/// data required for this device.
////////////////////////////////////////////////////////////////////////////////
SignalValue CDevice::GetState(std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !HasState(signal) )
    {
        Logger.Error << "Bad Device State: " << signal << "\n" << m_devid
                << "\n" << m_devinfo << std::endl;
        throw std::runtime_error("Bad Device State: " + signal);
    }

    return m_adapter->GetState(m_devid, signal);
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the set of recognized state signals.
///
/// @pre None.
/// @post Returns m_devinfo.s_state.
/// @return The set of recognized state signals.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::set<std::string> CDevice::GetStateSet() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_state;
}

////////////////////////////////////////////////////////////////////////////////
/// Accessor for the set of recognized command signals.
///
/// @pre None.
/// @post Returns m_devinfo.s_command.
/// @return The set of recognized command signals.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::set<std::string> CDevice::GetCommandSet() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_command;
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the value of a device command in the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if CDevice::HasCommand returns
/// false for the passed string identifier.
/// @pre The device must recognize the passed signal.
/// @post m_adapter is queried to set the value of the passed signal.
/// @param signal The string identifier for the command signal to set.
/// @param value The value to set for the command signal.
///
/// @limitations This function can fail even if CDevice::HasCommand returns
/// true. This case happens when m_adapter is not configured to store all the
/// data required for this device.
////////////////////////////////////////////////////////////////////////////////
void CDevice::SetCommand(std::string signal, SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !HasCommand(signal) )
    {
        Logger.Error << "Bad Device Command: " << signal << "\n" << m_devid
                << "\n" << m_devinfo << std::endl;
        throw std::runtime_error("Bad Device Command: " + signal);
    }

    m_adapter->SetCommand(m_devid, signal, value);
}

} // namespace device
} // namespace broker
} // namespace freedm
