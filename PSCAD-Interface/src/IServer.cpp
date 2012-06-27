///////////////////////////////////////////////////////////////////////////////
/// @file         IServer.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  TCP server that handles a single client at a time
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

#include "IServer.hpp"
#include "CLogger.hpp"

#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>

namespace freedm {
namespace simulation {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new TCPv4 server that listens on a given port number.
/// @pre The specified port number must be available to use.
/// @post Initiates a TCPv4 server on the specified port number.
/// @param port The port number to use for the TCP server.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
IServer::IServer( unsigned short port )
    : m_acceptor(m_service)
    , m_socket(m_service)
    , m_id(port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    using boost::asio::ip::tcp;
    tcp::endpoint endpoint( tcp::v4(), port );
    
    m_acceptor.open( endpoint.protocol() );
    m_acceptor.set_option( tcp::acceptor::reuse_address(true) );
    m_acceptor.bind( endpoint );
    m_acceptor.listen();
    
    Logger.Status << "Opened TCP server on port " << port << "." << std::endl;
    
    StartAccept();
}

///////////////////////////////////////////////////////////////////////////////
/// Runs the io_service and blocks the current thread.
/// @pre None.
/// @post Blocks the current thread until IServer::Stop() is called.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void IServer::Run()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Status << "Blocking thread with io_service::run()." << std::endl;
    m_service.run();
}

///////////////////////////////////////////////////////////////////////////////
/// Stops the io_service and unblocks the current thread.
/// @pre None.
/// @post Interrupts the TCP server and unblocks the current thread.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void IServer::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_service.stop();
    Logger.Status << "Thread unblocked with io_service::stop()." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Closes both the acceptor and the socket prior to destruction.
/// @pre None.
/// @post m_acceptor and m_socket are closed in that order.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
IServer::~IServer()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_acceptor.is_open() )
    {
        Logger.Warn << "Closed the acceptor." << std::endl;
        m_acceptor.close();
    }
    if( m_socket.is_open() )
    {
        Logger.Warn << "Closed an open client connection." << std::endl;
        m_socket.close();
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Schedules the next client connection.
/// @pre None.
/// @post Closes the current client connection on m_socket.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void IServer::StartAccept()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_socket.is_open() )
    {
        Logger.Info << "Closed an open client connection." << std::endl;
        m_socket.close();
    }
    m_acceptor.async_accept( m_socket, boost::bind( &IServer::HandleAccept,
            this, boost::asio::placeholders::error) );
}

///////////////////////////////////////////////////////////////////////////////
/// Redirects an accepted client to the connection handler.
/// @pre None.
/// @post Schedules the next connection with IServer::StartAccept()
/// @param error The io_service error code if the connection failed.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void IServer::HandleAccept( const boost::system::error_code & error )
{
    if( !error )
    {
        Logger.Info << "Accepted a new client connection." << std::endl;
        HandleConnection();
    }
    else
    {
        Logger.Warn << "Failed to accept a client connection" << std::endl;
    }
    StartAccept();
}

} // namespace simulation
} // namespace freedm
