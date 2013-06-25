////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceDesd.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Represents a distributed energy storage device.
///
/// @functions
///     CDeviceDesd::CDeviceDesd
///     CDeviceDesd::~CDeviceDesd
///     CDeviceDesd::Create
///     CDeviceDesd::GetStorage
///     CDeviceDesd::StepStorage
///
/// @functions
///     DeviceDesd::CDeviceDesd
///     DeviceDesd::~CDeviceDesd
///     DeviceDesd::GetStorage
///     DeviceDesd::StepStorage
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

#include "CDeviceDesd.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs the DESD.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceDesd::CDeviceDesd(const std::string identifier,
        IAdapter::Pointer adapter)
    : IDevice(identifier, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_StateSet.insert("storage");
    m_CommandSet.insert("storage");
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceDesd::~CDeviceDesd()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs another DESD.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for the device.
/// @return shared pointer to the new device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::Pointer CDeviceDesd::Create(const std::string identifier,
        IAdapter::Pointer adapter) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return IDevice::Pointer(new CDeviceDesd(identifier, adapter));
}

////////////////////////////////////////////////////////////////////////////////
/// Determine how much energy is stored by the DESD.
///
/// @pre None.
/// @post Calls IAdapter::Get with the signal "storage".
/// @return The amount of energy stored in this DESD.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SignalValue CDeviceDesd::GetStorage() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("storage");
}

////////////////////////////////////////////////////////////////////////////////
/// Increases the stored energy of the DESD by the passed amount.
///
/// @pre None.
/// @post Determines the current storage with CDeviceDesd::GetStorage.
/// @post Calls IAdapter::Set with the signal "storage".
/// @param step The amount to add to the current storage.
///
/// @limitations The storage increase will take some time to manifest.
////////////////////////////////////////////////////////////////////////////////
void CDeviceDesd::StepStorage(const SignalValue step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Set("storage", GetStorage() + step);
}

} // namespace device
} // namespace broker
} // namespace freedm
