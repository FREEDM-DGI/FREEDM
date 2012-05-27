////////////////////////////////////////////////////////////////////////////////
/// @file       CDeviceDesd.cpp
///
/// @author     Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Represents a distributed energy storage device.
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

#include "CLogger.hpp"
#include "device/types/CDeviceDesd.hpp"

namespace freedm {
namespace broker {
namespace device {

static CLocalLogger Logger(__FILE__);

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDesd::CDeviceDesd(Identifier, IPhysicalAdapter::AdapterPtr)
///
/// @description Instantiates a device.
///
/// @param device The unique device identifier for the device.
/// @param adapter The adapter that implements operations for this device.
////////////////////////////////////////////////////////////////////////////////
CDeviceDesd::CDeviceDesd(const Identifier device,
        IPhysicalAdapter::Pointer adapter)
: IDevice(device, adapter)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDesd::~CDeviceDesd()
///
/// @description Virtual destructor for derived classes.
////////////////////////////////////////////////////////////////////////////////
CDeviceDesd::~CDeviceDesd()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceSst::GetStorage() const
///
/// @description Determine how much energy is stored by this DESD.
///
/// @return The amount of energy stored in this DESD.
////////////////////////////////////////////////////////////////////////////////
SettingValue CDeviceDesd::GetStorage() const
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    return Get("storage");
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDesd::StepStorage(const SettingValue)
///
/// @description Increases the stored energy of this SST by the amount step.
///
/// @pre None.
/// @post The storage has been increased by step.
////////////////////////////////////////////////////////////////////////////////
void CDeviceDesd::StepStorage(const SettingValue step)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    Set("storage", GetStorage() + step);
}

}
}
}
