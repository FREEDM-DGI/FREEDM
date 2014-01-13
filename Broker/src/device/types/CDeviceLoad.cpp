////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceLoad.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Represents a load.
///
/// @functions
///     CDeviceLoad::CDeviceLoad
///     CDeviceLoad::~CDeviceLoad
///     CDeviceLoad::Create
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
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceLoad::CDeviceLoad(const std::string identifier,
        IAdapter::Pointer adapter)
    : IDevice(identifier, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_StateSet.insert("drain");
    m_CommandSet.insert("drain");
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
/// Constructs another load.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for the device.
/// @return shared pointer to the new device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::Pointer CDeviceLoad::Create(const std::string identifier,
        IAdapter::Pointer adapter) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return IDevice::Pointer(new CDeviceLoad(identifier, adapter));
}

////////////////////////////////////////////////////////////////////////////////
/// Determines the energy drain of the load.
///
/// @pre None.
/// @post Calls IAdapter::Get with the signal "drain".
/// @return The energy drain of this load.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SignalValue CDeviceLoad::GetLoad() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("drain");
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the amount of energy drain of this load.
///
/// @pre None.
/// @post Determines the current drain with CDeviceLoad::GetLoad.
/// @post Calls IAdapter::Set with the signal "load".
/// @param load The new amount of energy drain.
///
/// @limitations The energy drain increase will take some time to manifest.
////////////////////////////////////////////////////////////////////////////////
void CDeviceLoad::SetLoad(const SignalValue load)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Set("drain", load);
}

} // namespace device
} // namespace broker
} // namespace freedm
