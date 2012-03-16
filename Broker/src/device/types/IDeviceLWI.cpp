////////////////////////////////////////////////////////////////////////////////
/// @file           IDeviceLWI.cpp
///
/// @author         Yaxi Liu <ylztf@mst.edu>
///                 Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Physical devices for the LWI project
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
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "IDeviceLWI.hpp"

namespace freedm {
namespace broker {
namespace device {

////////////////////////////////////////////////////////////////////////////////
/// turnOn()
/// @description Turns the device on
////////////////////////////////////////////////////////////////////////////////
void IDeviceLWI::turnOn()
{
    Set("onOffSwitch",1);
}

////////////////////////////////////////////////////////////////////////////////
/// turnOff()
/// @description Turns the device off
////////////////////////////////////////////////////////////////////////////////
void IDeviceLWI::turnOff()
{
    Set("onOffSwitch",0);
}

////////////////////////////////////////////////////////////////////////////////
/// get_powerLevel()
/// @description Gets the power level of the device
/// @return Current power level stored in the device structure
////////////////////////////////////////////////////////////////////////////////
SettingValue IDeviceLWI::get_powerLevel()
{
    return Get("powerLevel");
}

} // namespace device
} // namespace broker
} // namespace freedm
