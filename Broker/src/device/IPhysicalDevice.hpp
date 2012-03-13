////////////////////////////////////////////////////////////////////////////////
/// @file           IPhysicalDevice.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Abstract base classes for physical devices.
///
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
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef I_PHYSICAL_DEVICE_HPP
#define I_PHYSICAL_DEVICE_HPP

#include <string>
#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Type of the unique device identifier
typedef std::string Identifier;

/// Type of the key for device settings
typedef std::string SettingKey;

/// Type of the value for device settings
typedef double SettingValue;

/// Defines the interface of the device get value function
class IDeviceGet
{
public:
    /// Virtual destructor for derived classes
    virtual ~IDeviceGet() {}

    /// Handle the device get value operation.
    virtual SettingValue Get( const SettingKey & key ) = 0;
};

/// Defines the interface of the device set value function
class IDeviceSet
{
public:
    /// Virtual destructor for derived classes
    virtual ~IDeviceSet() {}

    /// Handle the device set operation.
    virtual void Set( const SettingKey & key, const SettingValue & value ) = 0;
};

/// Defines the interface of the device implementation scheme
class IDeviceStructure
    : public IDeviceGet
    , public IDeviceSet
{
public:
    /// Convenience type for a shared pointer to self
    typedef boost::shared_ptr<IDeviceStructure> DevicePtr;

    /// Virtual destructor for derived classes
    virtual ~IDeviceStructure() {}

    /// Registers a device identifier with the structure
    void Register( const Identifier & devid ) { m_device = devid; }

    /// Returns the device registered with the structure
    Identifier GetDevice() const { return m_device; }
private:
    /// Identifies the device that owns the structure
    Identifier m_device;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // I_PHYSICAL_DEVICE_HPP
