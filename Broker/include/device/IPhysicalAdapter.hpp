////////////////////////////////////////////////////////////////////////////////
/// @file           IPhysicalAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>,
///                 Yaxi Liu <ylztf@mst.edu>,
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter.
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

#ifndef IPHYSICALADAPTER_HPP
#define	IPHYSICALADAPTER_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <string>

namespace freedm {
namespace broker {
namespace device {

/// Type of the unique device identifier
typedef std::string Identifier;

/// Type of the key for device settings
typedef std::string SettingKey;

/// Type of the value for device settings
typedef float SettingValue;

////////////////////////////////////////////////////////////////////////////////
/// @brief  Physical adapter device interface.
///
/// @description
///     Physical adapter device interface. Each device contains a reference to
///     an adapter that it uses to perform all operations. The adapter is
///     responsible for implementing the behavior of "get value" and "set value" 
///     operations on devices. The adapter is, in effect, the device's "driver".
///     Note that the same adapter can be used for all devices in the
///     simulation if this is desirable.
////////////////////////////////////////////////////////////////////////////////
class IPhysicalAdapter : private boost::noncopyable
{
public:
    /// Pointer to a physical adapter.
    typedef boost::shared_ptr<IPhysicalAdapter> AdapterPointer;

    /// Retrieves a value from a device.
    virtual SettingValue Get(const Identifier device,
            const SettingKey key) const = 0;

    /// Sets a value on a device.
    virtual void Set(const Identifier device, const SettingKey key,
            const SettingValue value) = 0;

    /// Virtual destructor for derived classes.
    virtual ~IPhysicalAdapter() { };
};

}
}
}

#endif	/* IPHYSICALADAPTER_HPP */

