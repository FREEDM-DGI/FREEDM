////////////////////////////////////////////////////////////////////////////////
/// @file           CTcpServer.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    TCP server that accepts a single client connection.
///
/// @functions
///     CTcpServer::CTcpServer
///     CTcpServer::~CTcpServer
///     CTcpServer::Create
///     CTcpServer::RegisterHandler
///     CTcpServer::GetPort
///     CTcpServer::GetHostname
///     CTcpServer::StartAccept
///     CTcpServer::HandleAccept
///     CTcpServer::hdr
///     
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

#include "CTcpServer.hpp"
#include "CLogger.hpp"

#include <string>
#include <stdexcept>

#include <boost/bind.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor for a TCP server.
///
/// @pre The specified port number must be valid.
/// @post Constructs a TCP server that accepts connections on the given port.
/// @param ios The I/O service used by the server.
/// @param port The listen port of the server.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CTcpServer::CTcpServer(boost::asio::io_service & ios, unsigned short port)
    : m_acceptor(ios)
    , m_socket(ios)
    , m_port(port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
   
    using boost::asio::ip::tcp; 
    tcp::endpoint endpoint(tcp::v4(), port);

    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();

    Logger.Status << "Opened TCP server on port " << port << "." << std::endl;

    StartAccept();
}

////////////////////////////////////////////////////////////////////////////////
/// Closes the acceptor and socket prior to destruction.
///
/// @pre None.
/// @post m_acceptor and m_socket are closed.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CTcpServer::~CTcpServer()
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;
    Stop();
}

void CTcpServer::Stop()
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_acceptor.is_open() )
    {
        Logger.Info << hdr() << "Closed TCP server acceptor." << std::endl;
        m_acceptor.close();
    }

    if( m_socket.is_open() )
    {
        Logger.Warn << hdr() << "Closed open client connection." << std::endl;
        m_socket.close();
    }
    
    Logger.Status << "Closed TCP server on port " << m_port << "." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new TCP server.
///
/// @pre The specified port number must be valid.
/// @post Creates a TCP server that accepts connections on the given port.
/// @param ios The I/O service used by the server.
/// @param port The listen port of the server.
/// @return Shared pointer to the server.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CTcpServer::Pointer CTcpServer::Create(boost::asio::io_service & ios,
        unsigned short port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Pointer(new CTcpServer(ios, port));
}

////////////////////////////////////////////////////////////////////////////////
/// Registers a client connection handler with the server.
///
/// @ErrorHanlding Throws a std::runtime_error if either m_handler has already
/// been initialized or the passed function is null.
/// @pre m_handler must not be initialized.
/// @post Assigns the passed function to m_handler.
/// @param h The callback function to handle client connections.
///
/// @limitations This function can only be called once.
////////////////////////////////////////////////////////////////////////////////
void CTcpServer::RegisterHandler(ConnectionHandler h)
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;

    if( !m_handler.empty() )
    {
        throw std::runtime_error(hdr() + "Cannot override client handler.");
    }

    if( h.empty() )
    {
        throw std::runtime_error(hdr() + "Cannot use null client handler.");
    }

    m_handler = h;
    
    Logger.Notice << hdr() << "Set client connection handler." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the listen port of the TCP server.
///
/// @pre None.
/// @post Returns the value of m_port.
/// @return The port number for the TCP server.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
unsigned short CTcpServer::GetPort() const
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;
    return m_port;
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the hostname of the open socket connection.
///
/// @ErrorHandling Throws a std::runtime_error if there is no open connection.
/// @pre m_socket must have an open connection prior to this call.
/// @post Queries m_socket for the hostname of its endpoint.
/// @return The hostname of the current client.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string CTcpServer::GetHostname() const
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;
    
    if( !m_socket.is_open() )
    {
        throw std::runtime_error(hdr() + "No open client connection.");
    }
    
    return m_socket.remote_endpoint().address().to_string();
}

////////////////////////////////////////////////////////////////////////////////
/// Prepares to accept the next client connection.
///
/// @pre None.
/// @post Closes the current client connection on m_socket.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CTcpServer::StartAccept()
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;

    if( m_socket.is_open() )
    {
        Logger.Warn << hdr() << "Closed open client connection." << std::endl;
        m_socket.close();
    }
    if( m_acceptor.is_open() )
    {
        m_acceptor.async_accept(m_socket, boost::bind(&CTcpServer::HandleAccept,
                this, boost::asio::placeholders::error));

        Logger.Info << hdr() << "Waiting for next connection." << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Redirects an accepted client to the connection handler.
///
/// @ErrorHandling Throws a std::runtime_error if the connection handler has
/// not been defined with CTcpServer::RegisterHandler.
/// @pre CTcpServer::RegisterHandler must be called prior to this function.
/// @post Calls m_handler to handle the client connection.
/// @post Schedules the next client with CTcpServer::StartAccept.
/// @param error The error code if the connection failed.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CTcpServer::HandleAccept(const boost::system::error_code & error)
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;

    if( !error )
    {
        Logger.Info << hdr() << "Accepted new client connection." << std::endl;
        
        if( m_handler.empty() )
        {
            throw std::runtime_error(hdr() + "Null connection handler.");
        }
        m_handler();
    }
    else
    {
        Logger.Warn << hdr() << "Failed to accept a client." << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Gets a log header for this object.
///
/// @pre None.
/// @post A string "(m_port)" unique to this server.
/// @return A header to use with the logger.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string CTcpServer::hdr() const
{
    return "(" + boost::lexical_cast<std::string>(m_port) + ") ";
}

boost::asio::ip::tcp::socket & CTcpServer::GetSocket()
{
    if( !m_socket.is_open() )
    {
        throw std::runtime_error("socket not open");
    }
    
    return m_socket;
}

} // namespace device
} // namespace broker
} // namespace freedm

