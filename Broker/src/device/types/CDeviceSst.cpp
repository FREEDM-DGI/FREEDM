////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceSst.cpp
///
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Represents a solid state transformer.
///
/// @functions
///     CDeviceSst::CDeviceSst
///     CDeviceSst::~CDeviceSst
///     CDeviceSst::Create
///     CDeviceSst::GetGateway
///     CDeviceSst::StepGateway
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

#include "CDeviceSst.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs the SST.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceSst::CDeviceSst(const std::string identifier, IAdapter::Pointer adapter)
    : IDevice(identifier, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_StateSet.insert("gateway");
    m_CommandSet.insert("gateway");
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceSst::~CDeviceSst()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs another SST.
///
/// @pre None.
/// @post Constructs a new device.
/// @param identifier The unique identifier for the device.
/// @param adapter The adapter that implements operations for the device.
/// @return shared pointer to the new device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDevice::Pointer CDeviceSst::Create(const std::string identifier,
        IAdapter::Pointer adapter) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return IDevice::Pointer(new CDeviceSst(identifier, adapter));
}

////////////////////////////////////////////////////////////////////////////////
/// Retrieve the gateway value of the SST.
///
/// @pre None.
/// @post Calls IAdapter::Get with the signal "gateway".
/// @return The gateway of this SST.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SignalValue CDeviceSst::GetGateway() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("gateway");
}

////////////////////////////////////////////////////////////////////////////////
/// Increases the gateway value of this SST by the passed amount.
///
/// @pre None.
/// @post Determines the current gateway with CDeviceSst::GetGateway.
/// @post Calls IAdapter::Set with the signal "storage".
/// @param v The amount to set to the current gateway.
///
/// @limitations The gateway increase will take some time to manifest.
////////////////////////////////////////////////////////////////////////////////
void CDeviceSst::SetGateway(const SignalValue v)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Set("gateway", v);
}

} // namespace device
} // namespace broker
} // namespace freedm
