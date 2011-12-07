////////////////////////////////////////////////////////////////////////////////
/// @file           CDevice.hpp
///
/// @author         Stephen Jackson <scj7t4@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Physical device class with variable implementation
///
/// @license
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_HPP
#define C_DEVICE_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "IPhysicalDevice.hpp"

namespace freedm {
namespace broker {

// forward declaration of device manager
class CPhysicalDeviceManager;

namespace device {

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
class CDevice
    : public IDeviceGet
    , public IDeviceSet
    , private boost::noncopyable
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<CDevice> DevicePtr;
    
    /// Constructor which takes a manager, identifier, and internal structure
    CDevice( CPhysicalDeviceManager & manager, Identifier device,
        IDeviceStructure::DevicePtr structure );
    
    /// Virtual destructor for derived classes
    virtual ~CDevice() {}
    
    /// Gets the setting of some key from the structure
    virtual SettingValue Get( const SettingKey & key );
    
    /// Sets the value of some key in the structure
    virtual void Set( const SettingKey & key, const SettingValue & value );
    
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
    const Identifier & GetID() const;
protected:
    /// Device manager that handles the device
    CPhysicalDeviceManager & m_manager;
    
    /// Mutex to protect the device from other threads
    mutable boost::mutex m_mutex;
    
    /// Unique identifier for the device
    Identifier m_device;
    
    /// Structure that handles the device data
    IDeviceStructure::DevicePtr m_structure;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_HPP
