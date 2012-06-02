////////////////////////////////////////////////////////////////////////////////
/// @file       CDeviceLoad.cpp
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
#include "device/types/CDeviceLoad.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceLoad::CDeviceLoad(Identifier, IPhysicalAdapter::AdapterPtr)
///
/// @description Instantiates a device.
///
/// @param device The unique device identifier for the device.
/// @param adapter The adapter that implements operations for this device.
////////////////////////////////////////////////////////////////////////////////
CDeviceLoad::CDeviceLoad(const Identifier device,
        IPhysicalAdapter::Pointer adapter)
: IDevice(device, adapter)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceLoad::~CDeviceLoad()
///
/// @description Virtual destructor for derived classes.
////////////////////////////////////////////////////////////////////////////////
CDeviceLoad::~CDeviceLoad()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceLoad::GetDrain() const
///
/// @description Determines the energy drain of this load.
///
/// @return The energy drain of this load.
////////////////////////////////////////////////////////////////////////////////
SettingValue CDeviceLoad::GetLoad() const
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    return Get("drain");
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceLoad::StepDrain(const SettingValue)
///
/// @description Increases the energy drain of this load by the amount step.
///
/// @pre None.
/// @post The gateway has been increased by step.
////////////////////////////////////////////////////////////////////////////////
void CDeviceLoad::StepLoad(const SettingValue step)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    Set("drain", GetLoad() + step);
}

}
}
}
