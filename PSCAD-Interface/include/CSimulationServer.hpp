////////////////////////////////////////////////////////////////////////////////
/// @file           CSimulationServer.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @description
///     Implementation of the interface to a power simulation.
///
/// @functions
///     CSimulationServer::CSimulationServer( const string &, unsigned short )
///     CSimulationServer::Stop()
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

#ifndef C_SIMULATION_SERVER_HPP
#define C_SIMULATION_SERVER_HPP

#include <list>
#include <string>
#include <cstring>
#include <iostream>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "logger.hpp"
#include "CDeviceTable.hpp"
#include "CSimulationInterface.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace simulation {

////////////////////////////////////////////////////////////////////////////////
/// CSimulationServer
///
/// @description
///     Provides an interface between the power simulation and cyber controls.
///     The simulation maintains a state table of variable readings from the
///     power simulation and a command table of settings to issue to the
///     simulation. These tables are shared with each cyber interface.
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////////
class CSimulationServer : private boost::noncopyable
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// CSimulationServer( const string &, unsigned short )
    ///
    /// @description
    ///     Creates the simulation server and starts the cyber interfaces.
    ///
    /// @Shared_Memory
    ///     m_state and m_command shared with each member of m_interface
    ///
    /// @Error_Handling
    ///     An exception will be thrown if the given XML file has a bad format.
    ///
    /// @pre
    ///     p_xml has tags for 'command' and 'state'
    ///
    /// @post
    ///     m_interface is populated with cyber interfaces
    ///     simulation thread is created on CSimulationServer::Run()
    ///
    /// @param
    ///     p_xml is the filename of the XML input
    ///     p_port is the port the simulation connects to
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    CSimulationServer( const std::string & p_xml, unsigned short p_port );
    
    ////////////////////////////////////////////////////////////////////////////
    /// ~CSimulationServer
    ///
    /// @description
    ///     Blocks until the simulation thread has finished execution.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     none
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    ~CSimulationServer();
    
    ////////////////////////////////////////////////////////////////////////////
    /// Stop
    ///
    /// @description
    ///     Sets the termination flag and stops the I/O service.
    ///
    /// @Shared_Memory
    ///     m_service is interrupted
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     m_quit set to true
    ///     m_service is stopped
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Stop();
private:
    ////////////////////////////////////////////////////////////////////////////
    /// Run
    ///
    /// @description
    ///     Message handler for the simulation server.
    ///
    /// @Shared_Memory
    ///     m_command and m_state are accessed and modifier
    ///     private members of each element in m_interface are accessed
    ///
    /// @Error_Handling
    ///     Messages that cause exceptions are disgarded without interrupting
    ///     the simulation server.
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     none
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Run();
    
    // container of external cyber interfaces
    std::list<CSimulationInterface::TPointer> m_interface;
    
    // service used by all the sockets
    boost::asio::io_service m_service;
    
    // simulation port number
    unsigned short m_port;
    
    // commands issued to devices
    CDeviceTable m_command;
    
    // state readings from devices
    CDeviceTable m_state;
    
    // worker thread for the server
    boost::thread m_thread;
    
    // flag for termination
    bool m_quit;
    
    static const unsigned short HEADER_SIZE = 8;
};

} // namespace simulation
} // namespace freedm

#endif // C_SIMULATION_SERVER_HPP
