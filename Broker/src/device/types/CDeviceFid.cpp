////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceFid.cpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents an FID.
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
#include "device/types/CDeviceFid.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFid::CDeviceFid(Identifier, IPhysicalAdapter::AdapterPtr)
///
/// @description Instantiates a device.
///
/// @param device The unique device identifier for the device.
/// @param adapter The adapter that implements operations for this device.
////////////////////////////////////////////////////////////////////////////////
CDeviceFid::CDeviceFid(const Identifier device,
        IPhysicalAdapter::Pointer adapter)
: IDevice(device, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFid::~CDeviceFid()
///
/// @description Virtual destructor for derived classes.
////////////////////////////////////////////////////////////////////////////////
CDeviceFid::~CDeviceFid()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// CDeviceFid::IsActive() const
///
/// @description Determines if this FID is active.
///
/// @return True if this FID is active (good), or false otherwise (bad). 
////////////////////////////////////////////////////////////////////////////////
bool CDeviceFid::IsActive() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("state") == 1.0;  // if not exactly 1, then something is wrong.
}

}
}
}
