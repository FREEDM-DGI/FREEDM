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
/// Registers a new device signal as state information with the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if the device signal is invalid
/// or already registered with the adapter.
/// @pre The parameters must not be empty.
/// @pre The device signal must not already be registered.
/// @post m_StateInfo is updated to store the new device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
///
/// @limitations This function does not prevent multiple device signals from
/// using the same index, which may be undesired behavior.
////////////////////////////////////////////////////////////////////////////////
void IPhysicalAdapter::RegisterStateInfo(std::string device,
        std::string signal, std::size_t index )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    DeviceSignal devsig(device, signal);
    
    if( device.empty() || signal.empty() )
    {
        std::stringstream ss;
        ss << "Received an invalid device signal." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    if( m_StateInfo.count(devsig) > 0 )
    {
        std::stringstream ss;
        ss << "The device signal (" << device << "," << signal << ") is "
                << "already registered as state information." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    m_StateInfo.insert(std::pair<DeviceSignal, std::size_t>(devsig, index));
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter state information." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Registers a new device signal as command information with the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if the device signal is invalid
/// or already registered with the adapter.
/// @pre The parameters must not be empty.
/// @pre The device signal must not already be registered.
/// @post m_CommandInfo is updated to store the new device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
///
/// @limitations This function does not prevent multiple device signals from
/// using the same index, which may be undesired behavior.
////////////////////////////////////////////////////////////////////////////////
void IPhysicalAdapter::RegisterCommandInfo(std::string device,
        std::string signal, std::size_t index )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    DeviceSignal devsig(device, signal);
    
    if( device.empty() || signal.empty() )
    {
        std::stringstream ss;
        ss << "Received an invalid device signal." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    if( m_CommandInfo.count(devsig) > 0 )
    {
        std::stringstream ss;
        ss << "The device signal (" << device << "," << signal << ") is "
                << "already registered as command information." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    m_CommandInfo.insert(std::pair<DeviceSignal, std::size_t>(devsig, index));
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter command information." << std::endl;
}

} // namespace device
} // namespace broker
} // namespace freedm
