////////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceDrer.cpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a distributed renewable energy resource.
///
/// @functions
///     CDeviceDrer::CDeviceDrer
///     CDeviceDrer::~CDeviceDrer
///     CDeviceDrer::GetGeneration
///     CDeviceDrer::StepGeneration
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

#include "CDeviceDrer.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs the DRER.
///
/// @pre None.
/// @post Constructs a new device.
/// @param device The unique identifier for the device.
/// @param adapter The adapter that implements operations for this device.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceDrer::CDeviceDrer(std::string device, IAdapter::Pointer adapter)
    : IDevice(device, adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_commands.insert("generation");
    m_states.insert("generation");    
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destructs the object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CDeviceDrer::~CDeviceDrer()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Determines the energy generation of the DRER.
///
/// @pre None.
/// @post Calls IAdapter::Get with the signal "generation".
/// @return The energy generation of this DRER.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
SignalValue CDeviceDrer::GetGeneration() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Get("generation");
}

////////////////////////////////////////////////////////////////////////////////
/// Increases the energy generation of this DRER by the passed amount.
///
/// @pre None.
/// @post Determines the current generation with CDeviceDrer::GetGeneration.
/// @post Calls IAdapter::Set with the signal "generation".
/// @param step The amount to add to the current generation.
///
/// @limitations The generation increase will take some time to manifest.
////////////////////////////////////////////////////////////////////////////////
void CDeviceDrer::StepGeneration(const SignalValue step)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Set("generation", GetGeneration() + step);
}

} // namespace device
} // namespace broker
} // namespace freedm
