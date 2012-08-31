////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceLoad.cpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a load.
///
/// @functions
///     CDeviceLoad::CDeviceLoad
///     CDeviceLoad::~CDeviceLoad
///     CDeviceLoad::GetLoad
///     CDeviceLoad::StepLoad
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "CDeviceLoad.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs the load.
///
/// @pre None.
/// @post Constructs a new device.
/// @param device The unique identifier for the device.
/// @param adapter The adapter that implements operations for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceLoad::CDeviceLoad(std::string device, PhysicalAdapter::Pointer adapter)
    : IDevice(device, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceLoad::~CDeviceLoad()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Determines the energy drain of the load.
///
/// @pre None.
/// @post Calls PhysicalAdapter::Get with the signal "drain".
/// @return The energy drain of this load.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SettingValue CDeviceLoad::GetLoad() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("drain");
}

////////////////////////////////////////////////////////////////////////////////
/// Increases the energy drain of this load by the passed amount.
///
/// @pre None.
/// @post Determines the current drain with CDeviceLoad::GetLoad.
/// @post Calls PhysicalAdapter::Set with the signal "load".
/// @param step The amount to add to the current drain.
///
/// @limitations The energy drain increase will take some time to manifest.
////////////////////////////////////////////////////////////////////////////////
void CDeviceLoad::StepLoad(const SettingValue step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Set("drain", GetLoad() + step);
}

} // namespace device
} // namespace broker
} // namespace freedm
