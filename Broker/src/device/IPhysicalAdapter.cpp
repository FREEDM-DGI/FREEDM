////////////////////////////////////////////////////////////////////////////////
/// @file           IPhysicalAdapter.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter.
///
/// @functions
///     IPhysicalAdapter::RegisterStateInfo
///     IPhysicalAdapter::RegisterCommandInfo
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

#include "IPhysicalAdapter.hpp"
#include "CLogger.hpp"

#include <sstream>
#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
void IPhysicalAdapter::RegisterStateInfo(std::string device,
        std::string signal, std::size_t index )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( device.empty() || signal.empty() )
    {
        std::stringstream ss;
        ss << "Received an invalid device signal." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    // create the device signal
    
    // check if the device signal has already been stored (exception)
    
    // store the map entry
    
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter state information." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
void IPhysicalAdapter::RegisterCommandInfo(std::string device,
        std::string signal, std::size_t index )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( device.empty() || signal.empty() )
    {
        std::stringstream ss;
        ss << "Received an invalid device signal." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    // create the device signal
    
    // check if the device signal has already been stored (exception)
    
    // store the map entry
    
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter command information." << std::endl;
}

} // namespace device
} // namespace broker
} // namespace freedm

