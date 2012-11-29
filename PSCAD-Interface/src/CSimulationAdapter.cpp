///////////////////////////////////////////////////////////////////////////////
/// @file         CSimulationAdapter.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the PSCAD power simulation
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

#include "CSimulationAdapter.hpp"
#include "CTableManager.hpp"
#include "DeviceTable.hpp"

#include <vector>
#include <cstring>
#include <cstddef>

#include <boost/asio.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Calls its base class constructors on the given arguments.
/// @pre See IServer::IServer() and CAdapter::CAdapter().
/// @post Initializes the IServer and CAdapter base classes.
/// @param port The port number to use for the server.
/// @param tree The property tree specification of the adapter.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CSimulationAdapter::CSimulationAdapter( unsigned short port,
        const boost::property_tree::ptree & tree )
    : IServer(port)
    , CAdapter(tree)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Reads the packet header and calls an appropriate message handler.
/// @Peers Communicates through socket connection to the IServer client.
/// @ErrorHandling If the header does not have a recognized callback, then the
/// packet will be dropped with a warning message.  No exception is thrown.
/// @pre The client must sent HEADER_SIZE bytes of data as a header.
/// @pre The received header must be RST, SET, or GET.
/// @post A private message handler is called based on the header.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CSimulationAdapter::HandleConnection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    char header[HEADER_SIZE];
    
    Logger.Info << "Waiting for header from client." << std::endl;
    boost::asio::read( m_socket, boost::asio::buffer(header) );
    Logger.Info << "Received the '" << header << "' header." << std::endl;
    
    if( strcmp(header,"RST") == 0 )
    {
        SetSimulationState();
        CTableManager::UpdateTable(COMMAND_TABLE,STATE_TABLE);
    }
    else if( strcmp(header,"SET") == 0 )
    {
        SetSimulationState();
    }
    else if( strcmp(header,"GET") == 0 )
    {
        GetExternalCommand();
    }
    else
    {
        Logger.Warn << header << " is not a recognized header." << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Reads the packet payload and uses it to update the state table.
/// @Peers Communicates through socket connection to the IServer client.
/// @Peers Acquires a unique write lock on the state table.
/// @pre The client must send the amount of data specified in the XML file.
/// @post A write lock is acquired on the state table.
/// @post The state table is updated according to the XML specification.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CSimulationAdapter::SetSimulationState()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::vector<TSignalValue> state(m_StateDetails.size());
    
    Logger.Info << "Waiting for payload from client." << std::endl;
    boost::asio::read( m_socket, boost::asio::buffer(state) );
    Logger.Info << "Received client payload." << std::endl;
    
    CTableManager::TWriter lock = CTableManager::AsWriter(STATE_TABLE);
    for( std::size_t i = 0, n = m_StateDetails.size(); i < n; i++ )
    {
        lock->SetValue(m_StateDetails[i],state[i]);
        Logger.Debug << m_StateDetails[i] << "=" << state[i] << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Reads the command table and writes the content back to the client.
/// @Peers Communicates through socket connection to the IServer client.
/// @Peers Acquires a shared read lock on the command table.
/// @pre None.
/// @post A read lock is acquired on the command table.
/// @post Writes to the socket the information specified in the XML file.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CSimulationAdapter::GetExternalCommand()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::vector<TSignalValue> command(m_CommandDetails.size());
    
    CTableManager::TReader lock = CTableManager::AsReader(COMMAND_TABLE);
    
    for( std::size_t i = 0, n = m_CommandDetails.size(); i < n; i++ )
    {
        command[i] = lock->GetValue(m_CommandDetails[i]);
        Logger.Debug << "Retrieved " << m_CommandDetails[i] << std::endl;
    }
    boost::asio::write( m_socket, boost::asio::buffer(command) );
    Logger.Info << "Wrote response to client." << std::endl;
}

} // namespace adapter
} // namespace simulation
} // namespace freedm
