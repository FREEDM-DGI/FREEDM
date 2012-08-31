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

#include "../IAdapter.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace freedm {
namespace broker {
namespace device {

////////////////////////////////////////////////////////////////////////////////
/// device_cast<TargetType>( ObjectType )
/// @pre ObjectType must be a boost::shared_ptr<T> for some T
/// @param object The boost::shared_ptr<T> to convert to TargetType
/// @return boost::shared_ptr<TargetType> to object if conversion possible
/// @return empty boost::shared_ptr<TargetType> if conversion impossible
////////////////////////////////////////////////////////////////////////////////
template <class TargetType, class ObjectType>
boost::shared_ptr<TargetType> device_cast(ObjectType object)
{
    return boost::dynamic_pointer_cast<TargetType>( object );
}

/// Physical device with implementation delegated to private member
class IDevice : private boost::noncopyable
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<IDevice> Pointer;

    /// Virtual destructor for derived classes
    virtual ~IDevice();

    /// Gets the device identifier
    Identifier GetID() const;

    /// Acquires the mutex
    void Lock();

    /// Releases a mutex lock
    void Unlock();

    /// Tries to acquire the mutex
    bool TryLock();

    /// Gets the setting of some key from the structure
    SettingValue Get(const SettingKey key) const;

    /// Sets the value of some key in the structure
    void Set(const SettingKey key, const SettingValue value);
protected:
    /// @todo
    friend class CDeviceManager; // Temporary?
    
    /// Constructor which takes an identifier and device adapter
    IDevice(const Identifier device, IAdapter::Pointer adapter);

    /// Unique identifier for the device
    Identifier m_identifier;

    /// "Driver" that handles the device data
    IAdapter::Pointer m_adapter;
    
    /// Mutex to protect the device from other threads
    boost::mutex m_mutex;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif //I_DEVICE_HPP
