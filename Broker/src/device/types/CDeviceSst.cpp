////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceSst.cpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a distributed energy storage device.
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

#include "CLogger.hpp"
#include "device/types/CDeviceSst.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceSst::CDeviceSst(Identifier, IPhysicalAdapter::AdapterPtr)
///
/// @description Instantiates a device.
///
/// @param device The unique device identifier for the device.
/// @param adapter The adapter that implements operations for this device.
////////////////////////////////////////////////////////////////////////////////
CDeviceSst::CDeviceSst(const Identifier device,
        IPhysicalAdapter::Pointer adapter)
: IDevice(device, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceSst::~CDeviceSst()
///
/// @description Virtual destructor for derived classes.
////////////////////////////////////////////////////////////////////////////////
CDeviceSst::~CDeviceSst()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceSst::GetGateway() const
///
/// @description Retrieve the gateway value of this SST.
///
/// @return The gateway of this SST.
////////////////////////////////////////////////////////////////////////////////
SettingValue CDeviceSst::GetGateway() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("gateway");
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceSst::StepGateway(const SettingValue)
///
/// @description Increases the gateway value of this SST by the amount step.
///
/// @pre None.
/// @post The gateway has been increased by step.
////////////////////////////////////////////////////////////////////////////////
void CDeviceSst::StepGateway(const SettingValue step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Set("gateway", GetGateway() + step);
}

}
}
}
