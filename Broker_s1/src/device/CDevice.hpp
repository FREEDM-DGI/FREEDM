////////////////////////////////////////////////////////////////////////////////
/// @file         CDevice.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Defines the interface for physical devices.
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

#ifndef C_DEVICE_HPP
#define C_DEVICE_HPP

#include "IAdapter.hpp"

#include <set>
#include <iosfwd>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Stores the internal structure of a device object.
struct DeviceInfo
{
    /// Set of types a device recognizes.
    std::set<std::string> s_type;

    /// Set of state signals a device recognizes.
    std::set<std::string> s_state;

    /// Set of command signals a device recognizes.
    std::set<std::string> s_command;
};

/// Outputs the device information to the passed output stream.
std::ostream & operator<<(std::ostream & os, const DeviceInfo & info);

/// Defines the interface used to access physical hardware.
////////////////////////////////////////////////////////////////////////////////
/// The CDevice class provides the universal interface for how modules in the
/// DGI interact with physical hardware. Each device object has an internal
/// DeviceInfo structure that specifies which types, states, and commands the
/// device object recognizes. If the device object tries to access a state or
/// command not defined by its structure, it will throw a runtime exception.
/// The device structure must be defined when a device object is created and
/// cannot be changed after construction.
///
/// The storage for devices is handled by a separate IAdapter member variable.
/// A device object queries its associated adapter each time it needs to read
/// a state variable or update a command value. An adapter must be defined at
/// device construction and cannot be changed later.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CDevice
    : private boost::noncopyable
{
public:
    /// Pointer to physical device interface.
    typedef boost::shared_ptr<CDevice> Pointer;

    /// Constructs a device with a specific structure and adapter.
    CDevice(std::string id, DeviceInfo info, IAdapter::Pointer adapter);

    /// Gets the unique identifier for this device.
    std::string GetID() const;

    /// Checks if the device recognizes a type.
    bool HasType(std::string type) const;

    /// Checks if the device recognizes a state signal.
    bool HasState(std::string signal) const;

    /// Checks if the device recognizes a command signal.
    bool HasCommand(std::string signal) const;

    /// Gets the current state of some signal from the adapter.
    SignalValue GetState(std::string signal) const;

    /// Gets the set of state signals recognized by the device.
    std::set<std::string> GetStateSet() const;

    /// Gets the set of command signals recognized by the device.
    std::set<std::string> GetCommandSet() const;

    /// Sets the next command for some signal in the adapter.
    void SetCommand(std::string signal, SignalValue value);

private:
    /// Unique identifier for this device.
    std::string m_devid;

    /// Internal structure of this device.
    DeviceInfo m_devinfo;

    /// Adapter that handles the storage for this device.
    IAdapter::Pointer m_adapter;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_HPP
