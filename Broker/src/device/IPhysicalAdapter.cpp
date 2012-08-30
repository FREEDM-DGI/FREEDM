///////////////////////////////////////////////////////////////////////////////
/// @file           IPhysicalAdapter.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
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
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
/*
    This is here for demonstration purposes.  It should be placed in the
    CRtdsAdapter::Start() function in some form:
    
Start()
{
    // this is a bit of a weird proof - but it works
    // we know there is no value < 1 in the set, because it's unsigned and the
    // insert code prevents insertions of 0.
    // because sets are sorted in increasing order, we also know the last
    // element of the set is the largest element.
    // therefore, if the last element is equal to the size, since sets contain
    // no duplicate values, that must mean every integer from 1 to size is
    // stored in the set.
    // otherwise, the numbers must be non-consecutive.
    // the short of it: it works.
    if( *m_StateIndex.rbegin() != m_StateIndex.size() )
    {
        std::stringstream ss;
        ss << "The state indices are not consecutive integers." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    if( *m_CommandIndex.rbegin() != m_CommandIndex.size() )
    {
        std::stringstream ss;
        ss << "The command indices are not consecutive integers." << std::endl;
        throw std::runtime_error(ss.str());
    }
}
*/

////////////////////////////////////////////////////////////////////////////////
/// Registers a new device signal as state information with the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if the device signal is invalid
/// or already registered with the adapter.
/// @pre The parameters must not be empty.
/// @pre The device signal must not already be registered.
/// @pre The index must not be registered with another signal.
/// @post m_StateInfo is updated to store the new device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
///
/// @limitations None.
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
    
    if( index == 0 )
    {
        std::stringstream ss;
        ss << "The state index must be greater than 0." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    if( m_StateIndex.count(index) > 0 )
    {
        std::stringstream ss;
        ss << "The state index " << index << " is a duplicate." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    m_StateInfo.insert(index);
    m_StateInfo.insert(std::pair<DeviceSignal, std::size_t>(devsig, index));
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter state information." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Registers a new device signal as command information with the adapter.
///
/// @ErrorHandling Throws a std::runtime_error if the device signal is invalid
/// or already registered with the adapter.
/// @pre The parameters must not be empty.
/// @pre The device signal must not already be registered.
/// @pre The index must not be registered with another signal.
/// @post m_CommandInfo is updated to store the new device signal.
/// @param device The unique identifier of the device to register.
/// @param signal The signal of the device that will be registered.
/// @param index The numeric index associated with the device signal.
///
/// @limitations None.
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
    
    if( index == 0 )
    {
        std::stringstream ss;
        ss << "The command index must be greater than 0." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    if( m_CommandIndex.count(index) > 0 )
    {
        std::stringstream ss;
        ss << "The command index " << index << " is a duplicate." << std::endl;
        throw std::runtime_error(ss.str());
    }
    
    m_CommandIndex.insert(index);
    m_CommandInfo.insert(std::pair<DeviceSignal, std::size_t>(devsig, index));
    Logger.Info << "Registered the device signal (" << device << "," << signal
            << ") as adapter command information." << std::endl;
}

} // namespace device
} // namespace broker
} // namespace freedm
