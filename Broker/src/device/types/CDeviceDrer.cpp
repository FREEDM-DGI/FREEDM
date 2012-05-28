////////////////////////////////////////////////////////////////////////////////
/// @file       CDeviceDrer.cpp
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
#include "device/types/CDeviceDrer.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDrer::CDeviceDrer(Identifier, IPhysicalAdapter::AdapterPtr)
///
/// @description Instantiates a device.
///
/// @param device The unique device identifier for the device.
/// @param adapter The adapter that implements operations for this device.
////////////////////////////////////////////////////////////////////////////////
CDeviceDrer::CDeviceDrer(const Identifier device,
        IPhysicalAdapter::Pointer adapter)
: IDevice(device, adapter)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDrer::~CDeviceDrer()
///
/// @description Virtual destructor for derived classes.
////////////////////////////////////////////////////////////////////////////////
CDeviceDrer::~CDeviceDrer()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDrer::GetGeneration() const
///
/// @description Determines the energy generation of this DRER.
///
/// @return The energy generation of this DRER.
////////////////////////////////////////////////////////////////////////////////
SettingValue CDeviceDrer::GetGeneration() const
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    return Get("generation");
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceDrer::StepGeneration(const SettingValue)
///
/// @description Increases the energy generation of this DRER by step.
///
/// @pre None.
/// @post The energy generation has been increased by step.
////////////////////////////////////////////////////////////////////////////////
void CDeviceDrer::StepGeneration(const SettingValue step)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    Set("generation", GetGeneration() + step);
}

}
}
}
