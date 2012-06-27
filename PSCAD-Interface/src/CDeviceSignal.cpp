///////////////////////////////////////////////////////////////////////////////
/// @file         CDeviceSignal.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Unique identifier for simulation state variables
///
/// @copyright
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
///////////////////////////////////////////////////////////////////////////////

#include "CDeviceSignal.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace simulation {

namespace // unnamed
{
    /// file scope debug stream
    CLocalLogger Logger(__FILE__);
}


///////////////////////////////////////////////////////////////////////////////
/// Constructs an empty device signal tuple.
/// @pre None.
/// @post Member variables initialized to empty strings.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CDeviceSignal::CDeviceSignal()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs a device signal tuple from the passed values.
/// @pre None.
/// @post Member variables initialized to the passed values.
/// @param device The unique name of a simulation device.
/// @param signal A signal associated with the simulation device.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CDeviceSignal::CDeviceSignal( std::string device, std::string signal )
    : m_device(device)
    , m_signal(signal)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// A strict weak ordering comparison operator based on string comparison.
/// @pre None.
/// @post Compares using m_device. If equivalent, compares using m_signal.
/// @param lhs The left hand side operator of the comparison.
/// @param rhs The right hand side operator of the comparison.
/// @return The boolean result of the string comparison.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
bool operator<( const CDeviceSignal & lhs, const CDeviceSignal & rhs )
{
    return lhs.m_device < rhs.m_device
            || (lhs.m_device == rhs.m_device && lhs.m_signal < rhs.m_signal);
}

///////////////////////////////////////////////////////////////////////////////
/// Streams concise output of the device signal to the passed stream.
/// @pre None.
/// @post Streams "(m_device,m_signal)" with no newline character.
/// @param os The output stream to receive the device signal.
/// @param devsig The device signal to output to the stream.
/// @return A reference to the passed output stream.
/// @limitations The output buffer will not be flushed by this call.
///////////////////////////////////////////////////////////////////////////////
std::ostream & operator<<( std::ostream & os, const CDeviceSignal & devsig )
{
    return os << "(" << devsig.m_device << "," << devsig.m_signal << ")";
}

} // namespace simulation
} // namespace freedm
