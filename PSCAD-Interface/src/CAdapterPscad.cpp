///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapterPscad.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Adapter for the DGI-PSCAD interface
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

#include "CAdapterPscad.hpp"
#include "CTableManager.hpp"
#include "DeviceTable.hpp"

#include <sstream>
#include <iostream>
#include <iterator>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>

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
CAdapterPscad::CAdapterPscad( unsigned short port,
        const boost::property_tree::ptree & tree )
    : IServer(port)
    , CAdapter(tree)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Handles requests from the client until a quit message is received.
/// @Peers Communicates through socket communication with the IServer client.
/// @pre Requests must start with a one-word header and end with \r\n.
/// @post Sends requests to CAdapterPscad::HandleMessage() until it returns F.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterPscad::HandleConnection()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::asio::streambuf request;    
    std::istream requestStream(&request);
    std::string requestType, content;
    
    try
    {
        do
        {
            // read the entire request
            request.consume( request.size() );
            Logger.Info << "Waiting for next client request." << std::endl;
            boost::asio::read_until( m_socket, request, "\r\n" );
            
            // extract the header
            requestStream >> requestType;
            Logger.Info << "Using " << requestType << " as header." << std::endl;
            
            // extract the remaining data
            std::istreambuf_iterator<char> it(requestStream);
            std::istreambuf_iterator<char> end;
            content = std::string( it, end );
            Logger.Info << "Using " << content << " as content." << std::endl;
        } while( HandleMessage( requestType, content ) );
    }
    catch( boost::system::system_error & e )
    {
        Logger.Info << "Disgraceful client disconnection." << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Calls a message handler and writes a response to the client.
/// @Peers Communicates through socket communication with the IServer client.
/// @pre None.
/// @post Sends the request to an appropriate message handler.
/// @post Responds with the format "StatusCode Message ReturnValue\r\n".
/// @param type The header of the request.
/// @param content The payload of the request.
/// @return True if the client intends to send more requests, false otherwise.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
bool CAdapterPscad::HandleMessage( std::string type, std::string content )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::asio::streambuf response;
    std::ostream responseStream(&response);
    bool continueConnection = true;
    
    if( type == "SET" )
    {
        responseStream << SetExternalCommand(content);
    }
    else if( type == "GET" )
    {
        responseStream << GetSimulationState(content);
    }
    else if( type == "QUIT" )
    {
        continueConnection = false;
        responseStream << "200 OK\r\n";
        Logger.Info << "Handled request to end connection." << std::endl;
    }
    else
    {
        responseStream << "400 BADREQUEST\r\n";
        Logger.Warn << "Received invalid request " << type << "." << std::endl;
    }
    
    boost::asio::write( m_socket, response);
    return continueConnection;
}

///////////////////////////////////////////////////////////////////////////////
/// Message handler to update a value in the command table.
/// @ErrorHandling The call will return an error status message if the request
/// could not be handled.  It will not throw an exception.
/// @pre The payload should have the format "Device Signal Value".
/// @pre The command table should contain the specified value.
/// @post Obtains a write lock on the command table.
/// @post Updates the value of DeviceSignal in the command table.
/// @param content The payload of the client request.
/// @return A status message to be sent back to the client.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
std::string CAdapterPscad::SetExternalCommand( std::string content )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream ss(content);
    std::string device, signal, strval, result;
    TSignalValue value;
    
    ss >> device >> signal >> strval;
    CDeviceSignal devsig(device,signal);
    value = boost::lexical_cast<TSignalValue>(strval);
    Logger.Debug << "Device=" << device << ", Signal=" << signal
                 << ", Value=" << value << std::endl;
    
    try
    {
        CTableManager::TWriter lock = CTableManager::AsWriter(COMMAND_TABLE);
        lock->SetValue( devsig, value );
        result = "200 OK\r\n";
        Logger.Info << "Set " << devsig << " in command table." << std::endl;
    }
    catch( std::exception & e )
    {
        result = "404 ERROR NOTFOUND\r\n";
        Logger.Warn << "Failed to find " << devsig << "." << std::endl;
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// Message handler to retrieve a value from the state table.
/// @ErrorHandling The call will return an error status message if the request
/// could not be handled.  It will not throw an exception.
/// @pre The payload should have the format "Device Signal".
/// @pre The state table should contain the specified value.
/// @post Obtains a read lock on the state table.
/// @param content The payload of the client request.
/// @return A reponse message to be sent back to the client.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
std::string CAdapterPscad::GetSimulationState( std::string content )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream ss(content);
    std::string device, signal, result;
    TSignalValue value;
    
    ss >> device >> signal;
    CDeviceSignal devsig(device,signal);
    Logger.Debug << "Device=" << device << ", Signal=" << signal << std::endl;
    
    try
    {
        CTableManager::TReader lock = CTableManager::AsReader(STATE_TABLE);
        value = lock->GetValue(devsig);
        result = "200 OK " + boost::lexical_cast<std::string>(value) + "\r\n";
        Logger.Info << "Got " << devsig << " from state table." << std::endl;
    }
    catch( std::exception & e )
    {
        result = "404 ERROR NOTFOUND\r\n";
        Logger.Warn << "Failed to find " << devsig << "." << std::endl;
    }
    
    return result;
}

} // namespace adapter
} // namespace simulation
} // namespace freedm
