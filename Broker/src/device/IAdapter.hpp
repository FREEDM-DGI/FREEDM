////////////////////////////////////////////////////////////////////////////////
/// @file           IAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter.
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

#ifndef I_ADAPTER_HPP
#define	I_ADAPTER_HPP

#include <set>
#include <cmath>
#include <string>
#include <utility>

#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Type of the value for device signals.
typedef float SignalValue;

/// Sent by the DGI to indicate it knows nothing about the state of a device.
const SignalValue NULL_COMMAND = std::pow(10, 8);

/// Type of the unique identifier for device values.
typedef std::pair<const std::string, const std::string> DeviceSignal;

/// Physical adapter device interface.
////////////////////////////////////////////////////////////////////////////////
/// Defines the interface each device uses to perform its operations.  The
/// concrete adapter is responsible for implementation of both Get and Set
/// functions.
///
/// The adapter class is intended to be private to the device subsystem.  If
/// you want to access it from outside devices, you are doing something wrong.
///
/// @limitations Adapters must be shut down via IAdapter::Stop() exactly once
///              before they are destructed in order to ensure correct behavior.
///              An adapter that has been shut down while references to it still
///              exist is basically an empty shell: IAdapter::Get() is
///              guaranteed to return the same value that it did before the
///              adapter was stopped, and IAdapter::Set() is guaranteed to
///              silently fail.
////////////////////////////////////////////////////////////////////////////////
class IAdapter
    : private boost::noncopyable
{
public:
    /// Pointer to a physical adapter.
    typedef boost::shared_ptr<IAdapter> Pointer;

    /// Starts the adapter.
    virtual void Start() = 0;

    /// Stops the adapter.  Guaranteed to be thread-safe.
    virtual void Stop() = 0;

    /// Retrieves a value from a device.
    virtual SignalValue GetState(const std::string device,
            const std::string signal) const = 0;

    /// Sets a value on a device.
    virtual void SetCommand(const std::string device, const std::string signal,
            const SignalValue value) = 0;

    /// Virtual destructor for derived classes.
    virtual ~IAdapter();

    /// Register a device name with the adapter.
    void RegisterDevice(const std::string devid);

    /// Get the list of registered device names.
    std::set<std::string> GetDevices() const;

protected:
    /// Constructor
    IAdapter();

    /// Reveals devices in the device manager.
    void RevealDevices();

private:
    /// Set of registered device names.
    std::set<std::string> m_devices;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // I_ADAPTER_HPP
