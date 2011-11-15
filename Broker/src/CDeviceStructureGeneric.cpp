////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceStructureGeneric.cpp
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

#include "CDeviceStructureGeneric.hpp"

namespace freedm {
namespace broker {
namespace device {

////////////////////////////////////////////////////////////////////////////////
/// Get( const SettingKey & )
/// @brief Returns the value of some key in the register
/// @param key The key to retrieve from the register
/// @return SettingValue if found, 0.0 if key not defined
////////////////////////////////////////////////////////////////////////////////
SettingValue CDeviceStructureGeneric::Get( const SettingKey & key )
{
    std::map<SettingKey,SettingValue>::iterator ri = m_register.find(key);
    if(ri != m_register.end())
        return ri->second;
    return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
/// Set( const SettingKey &, const SettingValue & )
/// @brief Sets the value of some key in the register
/// @pre none
/// @post m_register[key] = value
/// @param key The key to set in the register
/// @param value The value to set the key
////////////////////////////////////////////////////////////////////////////////
void CDeviceStructureGeneric::Set( const SettingKey & key, const SettingValue & value )
{
    m_register[key] = value;
}

} // namespace device
} // namespace broker
} // namespace freedm
