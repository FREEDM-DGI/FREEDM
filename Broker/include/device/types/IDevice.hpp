////////////////////////////////////////////////////////////////////////////////
/// @file       IDevice.hpp
///
/// @author     Stephen Jackson <scj7t4@mst.edu>,
/// @author     Thomas Roth <tprfh7@mst.edu>,
/// @author     Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
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

#ifndef I_DEVICE_HPP
#define I_DEVICE_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "../IPhysicalAdapter.hpp"
 
namespace freedm {
namespace broker {
namespace device {

// forward declaration of device manager
class CPhysicalDeviceManager;

////////////////////////////////////////////////////////////////////////////////
/// device_cast<TargetType>( ObjectType )
/// @pre ObjectType must be a boost::shared_ptr<T> for some T
/// @param object The boost::shared_ptr<T> to convert to TargetType
/// @return boost::shared_ptr<TargetType> to object if conversion possible
/// @return empty boost::shared_ptr<TargetType> if conversion impossible
////////////////////////////////////////////////////////////////////////////////
template <class TargetType, class ObjectType>
boost::shared_ptr<TargetType> device_cast( ObjectType object )
{
    return boost::dynamic_pointer_cast<TargetType>(object);
}

/// Physical device with implementation delegated to private member
class IDevice
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<IDevice> DevicePtr;

    /// Virtual destructor for derived classes
    virtual ~IDevice() {}

    /// Gets the setting of some key from the structure
    virtual SettingValue Get( const SettingKey key );

    /// Sets the value of some key in the structure
    virtual void Set( const SettingKey key, const SettingValue value );

    /// Gets the device manager for the device
    CPhysicalDeviceManager & GetManager();

    /// Gets the device manager for the device
    const CPhysicalDeviceManager & GetManager() const;

    /// Acquires the mutex
    void Lock();

    /// Releases a mutex lock
    void Unlock();

    /// Tries to acquire the mutex
    bool TryLock();

    /// Gets the device identifier
    Identifier GetID() const;
    
protected:
    /// Constructor which takes a manager, identifier, and device adapter
    IDevice( CPhysicalDeviceManager & manager, Identifier device,
        IPhysicalAdapter & adapter );

    /// Device manager that handles the device
    CPhysicalDeviceManager & m_manager;

    /// Mutex to protect the device from other threads
    mutable boost::mutex m_mutex;

    /// Unique identifier for the device
    Identifier m_identifier;

    /// "Driver" that handles the device data
    IPhysicalAdapter & m_adapter;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif //I_DEVICE_HPP
