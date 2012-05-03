////////////////////////////////////////////////////////////////////////////////
/// @file           CGenericAdapter.hpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>,
///                 Stephen Jackson <scj7t4@mst.edu>,
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a generic physical device adapter.
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

#ifndef CGENERICADAPTER_HPP
#define	CGENERICADAPTER_HPP

#include <map>
#include <sstream>
#include <string>

#include "IPhysicalAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

////////////////////////////////////////////////////////////////////////////////
/// @brief  Physical adapter device interface that stores settings in itself.
///
/// @description
///     Physical adapter device interface that stores device settings in itself.
///     This class is used when no communication outside the DGI processes is
///     desired - i.e. when there is no PSCAD or RTDS simulation.
///
/// @limitations
///     None.
////////////////////////////////////////////////////////////////////////////////
class CGenericAdapter : public IPhysicalAdapter
{  
public:
    /// Retrieves a value from a device.
    virtual SettingValue Get(const Identifier device,
            const SettingKey key) const;

    /// Sets a value on a device.
    virtual void Set(const Identifier device, const SettingKey key,
            const SettingValue value);

private:
    /// Map of device setting keys to values.
    typedef std::map<SettingKey, SettingValue> KeyMap;
    
    /// Map of devices of KeyMaps.
    typedef std::map<Identifier, KeyMap> DeviceMap;
    
    /// Virtual destructor for derived classes.
    virtual ~CGenericAdapter() { }

    /// Registry of device keys and values.
    mutable DeviceMap m_registry;
};

}
}
}

#endif	/* CGENERICADAPTER_HPP */

