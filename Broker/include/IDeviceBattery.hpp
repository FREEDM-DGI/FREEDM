///////////////////////////////////////////////////////////////////////////////
/// @file       IBatteryDevice.hpp
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

#ifndef I_DEVICE_BATTERY_HPP
#define I_DEVICE_BATTERY_HPP

#include <boost/shared_ptr.hpp>

#include "IPhysicalDevice.hpp"
#include "PhysicalDeviceTypesObsolete.hpp"

namespace freedm {
namespace broker {

class IDeviceBattery : virtual public IPhysicalDevice
{
public:
    /// Shared pointer to an instance of a battery device
    typedef boost::shared_ptr<IDeviceBattery> DevicePtr;
    
        /// Constructor which takes in the manager and device id.
    IDeviceBattery(CPhysicalDeviceManager& phymanager, Identifier deviceid)
        : IPhysicalDevice(phymanager,deviceid,physicaldevices::DESD) {}
    
    /// get the battery output power (positive values indicate discharge)
    virtual SettingValue get_powerLevel();
    
    /// activate the battery
    virtual void turnOn();
    
    /// deactivate the battery
    virtual void turnOff();
private:
};

} // namespace broker
} // namespace freedm

#endif // I_DEVICE_BATTERY_HPP
