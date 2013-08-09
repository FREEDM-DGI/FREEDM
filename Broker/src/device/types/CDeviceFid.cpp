////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFid.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Represents a fault interruption device.
///
/// @functions
///     CDeviceFid::CDeviceFid
///     CDeviceFid::~CDeviceFid
///     CDeviceFid::Create
///     CDeviceFid::IsActive
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

#include "CDeviceFid.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs the FID.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceFid::CDeviceFid(const std::string identifier, IAdapter::Pointer adapter)
    : IDevice(identifier, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_StateSet.insert("state");
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceFid::~CDeviceFid()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs another FID.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for the device.
/// @return shared pointer to the new device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::Pointer CDeviceFid::Create(const std::string identifier,
        IAdapter::Pointer adapter) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return IDevice::Pointer(new CDeviceFid(identifier, adapter));
}

////////////////////////////////////////////////////////////////////////////////
/// Determines if the FID is active.
///
/// @pre None.
/// @post Calls IAdapter::Get with the signal "state".
/// @return True if this FID is active (good), or false otherwise (bad). 
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
bool CDeviceFid::IsActive() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("state") == 1.0;  // if not exactly 1, then something is wrong.
}

} // namespace device
} // namespace broker
} // namespace freedm
