////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceStructureGeneric.hpp
///
/// @author         Stephen Jackson <scj7t4@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Generic physical device driver
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

#ifndef C_DEVICE_STRUCTURE_GENERIC_HPP
#define C_DEVICE_STRUCTURE_GENERIC_HPP

#include <map>

#include "IPhysicalDevice.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Generic device with a register to store device settings
class CDeviceStructureGeneric
    : public IDeviceStructure
{
public:
    /// Gets the setting of some key from the register
    virtual SettingValue Get( const SettingKey & key );
    
    /// Sets the value of some key in the register
    virtual void Set( const SettingKey & key, const SettingValue & value );
private:
    /// Register of current device settings
    std::map<SettingKey,SettingValue> m_register;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_STRUCTURE_GENERIC_HPP
