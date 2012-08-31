///////////////////////////////////////////////////////////////////////////////
/// @file           IBufferAdapter.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter.
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

#include "IBufferAdapter.hpp"

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

////////////////////////////////////////////////////////////////////////////
/// Set
///
/// @description
///     Search the cmdTable and then update the specified value.
///
/// @Error_Handling
///     Throws an exception if the device/key pair does not exist in the table.
///
/// @pre
///     none
///
/// @param
///     device is the unique identifier of a physical device (such as SST or 
///     Load)
///
///     key is the name of a feature of the device that can be maniputed
///     (such as onOffSwitch, chargeLevel, etc.)
///
///     value is the desired new setting
///
/// @limitations
///     RTDS uses floats. So value is type-cast into float from double.
///     There could be minor loss of accuracy.
///
////////////////////////////////////////////////////////////////////////////
void IBufferAdapter::Set(const Identifier device, const SettingKey key,
                         const SettingValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
         m_txBuffer[m_commandInfo[DeviceSignal(device, key)] = value;
    }
    catch( std::runtime_error & e )
    {
        std::stringstream ss;
        ss << "RTDS attempted to set device/key pair " << device << "/"
           << key << ", but this pair does not exist.";
        throw std::runtime_error(ss.str());
    }
}

////////////////////////////////////////////////////////////////////////////
/// Get
///
/// @description
///     Search the stateTable and read from it.
///
/// @Error_Handling
///     Throws an exception if the device/key pair does not exist in the table.
///
/// @pre
///     The socket connection has been established. Otherwise the numbers
///     read is junk.
///
/// @param
///     device is the unique identifier of a physical device (such as SST, 
///     Load)
///
///     key is a power electronic reading related to the device (such
///     as powerLevel, stateOfCharge, etc.)
///
/// @return
///     value from table is retrieved
///
/// @limitations
///     RTDS uses floats.  So the return value is type-cast into double from 
///     floats. The accuracy is not as high as a real doubles.
///
////////////////////////////////////////////////////////////////////////////
SettingValue IBufferAdapter::Get(const std::string device, 
                                 const std::string key) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try  
    {
        return m_txBuffer[m_commandInfo[DeviceSignal(device, key)];
    }
    catch( std::runtime_error & e )
    {
        std::stringstream ss;
        ss << "RTDS attempted to get device/key pair " << device << "/"
           << key << ", but this pair does not exist.";
        throw std::runtime_error(ss.str());
    }
}

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
void IBufferAdapter::RegisterStateInfo(const std::string device,
                                       const std::string signal,
                                       const std::size_t index )
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
void IBufferAdapter::RegisterCommandInfo(const std::string device,
                                         const std::string signal,
                                         const std::size_t index )
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
