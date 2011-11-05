///////////////////////////////////////////////////////////////////////////////
/// @file       IBatteryDevice.cpp
///
/// @author     Yaxi Liu <ylztf@mst.edu>
///             Thomas Roth <tprfh7@mst.edu>
///
/// @compiler   C++
///
/// @project    FREEDM DGI
///
/// @description The abstract base for physical batteries.
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missour University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
///////////////////////////////////////////////////////////////////////////////

#include "IDeviceBattery.hpp"

namespace freedm {
namespace broker {

IPhysicalDevice::SettingValue IDeviceBattery::get_powerLevel()
{
    return Get("powerlevel");
}

void IDeviceBattery::turnOn()
{
    Set( "onOffSwitch", 1 );
}

void IDeviceBattery::turnOff()
{
    Set( "onOffSwitch", 0 );
}

} // namespace broker
} // namespace freedm
