////////////////////////////////////////////////////////////////////////////////
/// @file         IDevice.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Physical device interface with variable implementations.
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

#ifndef I_DEVICE_HPP
#define I_DEVICE_HPP

#include "IAdapter.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace broker {
namespace device {

////////////////////////////////////////////////////////////////////////////////
/// Attempts to convert a device pointer into the specified device type.
///
/// @pre TargetType must be the type of some device class.
/// @pre ObjectType must be a boost::shared_ptr<T> for some T
/// @param object The boost::shared_ptr<T> to convert to TargetType
/// @return boost::shared_ptr<TargetType> to object if conversion is possible,
/// and an empty boost::shared_ptr<TargetType> if conversion is impossible.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <class TargetType, class ObjectType>
boost::shared_ptr<TargetType> device_cast(ObjectType object)
{
    return boost::dynamic_pointer_cast<TargetType>( object );
}

/// Physical device with implementation delegated to private member.
////////////////////////////////////////////////////////////////////////////////
/// The IDevice class provides the public interface used for all devices.  It
/// it has an associated set of device signals, defined by derived classes,
/// that can be accessed through the public IDevice::Get and IDevice::Set.
///
/// @limitations Thread safety must be handled by the adapter member.
////////////////////////////////////////////////////////////////////////////////
class IDevice
    : private boost::noncopyable
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<IDevice> Pointer;

    /// Virtual destructor for derived classes.
    virtual ~IDevice();

    /// Gets the unique device identifier.
    std::string GetID() const;

    /// Gets the value of some signal from the adapter.
    SignalValue Get(const std::string signal) const;

    /// Sets the value of some signal in the adapter.
    void Set(const std::string signal, const SignalValue value);
protected:
    /// Constructor which takes an identifier and device adapter.
    IDevice(const std::string device, IAdapter::Pointer adapter);

    /// Unique identifier for the device.
    std::string m_identifier;

    /// Handles the get and set requests.
    IAdapter::Pointer m_adapter;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif //I_DEVICE_HPP
