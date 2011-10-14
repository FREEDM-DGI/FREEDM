////////////////////////////////////////////////////////////////////////////////
/// @file           CSimulationInterface.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @description
///     Defines the interface between the simulation server and cyber control.
///
/// @functions
///     CSimulationInterface( io_service &, CDeviceTable &, CDeviceTable &, unsigned short, size_t )
///
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
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef C_SIMULATION_INTERFACE_CPP
#define C_SIMULATION_INTERFACE_CPP

#include <string>
#include <iostream>

#include <boost/ref.hpp>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "logger.hpp"
#include "CLineServer.hpp"
#include "CDeviceTable.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace simulation {

////////////////////////////////////////////////////////////////////////////////
/// CSimulationInterface
///
/// @description
///     Defines the interface between the cyber control and simulation server.
///     The interface provides shared access to the device tables maintained by
///     the simulation server and contains a line server that accepts requests
///     from cyber control algorithms.
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////////
class CSimulationInterface : private boost::noncopyable
{
public:
    typedef boost::shared_ptr<CSimulationInterface> TPointer;
    
    static TPointer Create( boost::asio::io_service & p_service, CDeviceTable & p_command,
        CDeviceTable & p_state, unsigned short p_port, size_t p_index );
private:
    ////////////////////////////////////////////////////////////////////////////
    /// CSimulationInterface( io_service &, CDeviceTable &, CDeviceTable &, unsigned short, size_t )
    ///
    /// @description
    ///     Creates a simulation interface using the given port number.
    ///
    /// @Shared_Memory
    ///     Uses the passed io_service and CDeviceTable until destroyed.
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     creates a new reference to a CLineServer on p_port
    ///
    /// @param
    ///     p_service is the io_service the line server runs on
    ///     p_command is the device command table maintained by the server
    ///     p_state is the device state table maintained by the server
    ///     p_port is the port the line server listens on
    ///     p_index is the interface unique identifier
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    CSimulationInterface( boost::asio::io_service & p_service, CDeviceTable & p_command,
        CDeviceTable & p_state, unsigned short p_port, size_t p_index );
    
    ////////////////////////////////////////////////////////////////////////////
    /// Set( const string &, const string &, const string & )
    ///
    /// @description
    ///     Modifies a value in the command table.
    ///
    /// @Shared_Memory
    ///     m_command is accessed and modified
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     (p_device,p_key) is in m_command
    ///     m_index has access to (p_device,p_key)
    ///
    /// @post
    ///     (p_device,p_key) in m_command is set to p_value
    ///
    /// @param
    ///     p_device is the device identifier to modify
    ///     p_key is the device variable to modify
    ///     p_value is the value to set
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Set( const std::string & p_device, const std::string & p_key, const std::string & p_value );
    
    ////////////////////////////////////////////////////////////////////////////
    /// Get( const string &, const string & )
    ///
    /// @description
    ///     Returns a value from the command table.
    ///
    /// @Shared_Memory
    ///     m_state is accessed
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     (p_device,p_key) is in m_state
    ///     m_index has access to (p_device,p_key)
    ///
    /// @post
    ///     none
    ///
    /// @param
    ///     p_device is the device identifier to retrieve
    ///     p_key is the device variable to retrieve
    ///
    /// @return
    ///     stored value for (p_device,p_key) in m_state
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    std::string Get( const std::string & p_device, const std::string & p_key );

    /// line server for cyber control requests
    CLineServer::TPointer m_server;
    
    /// device command table
    CDeviceTable & m_command;
    
    /// device state table
    CDeviceTable & m_state;
    
    /// unique identifier
    size_t m_index;
};

} // namespace simulation
} // namespace freedm

#endif // C_SIMULATION_INTERFACE_CPP
