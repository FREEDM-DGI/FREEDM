////////////////////////////////////////////////////////////////////////////////
/// @file           IDevice.hpp
///
/// @author         Stephen Jackson <scj7t4@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
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

#ifndef I_DEVICE_HPP
#define I_DEVICE_HPP

#include "IAdapter.hpp"

#include <set>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace broker {
namespace device {

////////////////////////////////////////////////////////////////////////////////
/// Attempts to convert a device pointer into the target device type.
///
/// @pre TargetType must be some class derived from IDevice.
/// @pre ObjectType must be a boost::shared_ptr<T> for some T.
/// @param object The boost::shared_ptr<T> to convert to TargetType.
/// @return boost::shared_ptr<TargetType> to object if conversion is possible,
/// or an empty boost::shared_ptr<TargetType> if conversion is impossible.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <class TargetType, class ObjectType>
boost::shared_ptr<TargetType> device_cast(ObjectType object)
{
    return boost::dynamic_pointer_cast<TargetType>(object);
}

/// Physical device with implementation delegated to a private adapter.
////////////////////////////////////////////////////////////////////////////////
/// The IDevice class provides the public interface used by devices.  It has an
/// associated set of device signals, defined by derived classes, which can be
/// accessed through the public IDevice::Get and IDevice::Set functions.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class IDevice
    : private boost::noncopyable
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<IDevice> Pointer;
    
    /// Virtual destructor for derived classes.
    virtual ~IDevice();
    
    /// Virtual constructor for derived classes.
    virtual Pointer Create(const std::string identifier,
            IAdapter::Pointer adapter) const = 0;
    
    /// Gets the device identifier.
    std::string GetID() const;
    
    /// Gets the value of some device signal.
    SignalValue Get(const std::string signal) const;
    
    /// Sets the value of some device signal.
    void Set(const std::string signal, const SignalValue value);
    
    /// Gets the set of recognized state signals.
    std::set<std::string> GetStateSet() const;
    
    /// Gets the set of recognized command signals.
    std::set<std::string> GetCommandSet() const;
    
    /// Checks if the device recognizes a state signal.
    bool HasStateSignal(const std::string signal) const;
    
    /// Checks if the device recognizes a command signal.
    bool HasCommandSignal(const std::string signal) const;
protected:
    /// Constructor for derived classes.
    IDevice(const std::string identifier, IAdapter::Pointer adapter);
    
    /// Unique device identifier.
    std::string m_identifier;
    
    /// Adapter that implements the get and set functions.
    IADapter::Pointer m_adapter;
    
    /// Set of state signals.
    std::set<std::string> m_StateSet;
    
    /// Set of command signals.
    std::set<std::string> m_CommandSet;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // I_DEVICE_HPP
