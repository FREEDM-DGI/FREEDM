////////////////////////////////////////////////////////////////////////////////
/// @file           CTcpServer.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    TCP server that accepts a single connection at a time.
///
/// @functions
///     CTcpServer::CTcpServer
///     CTcpServer::~CTcpServer
///     CTcpServer::Create
///     CTcpServer::ReceiveData
///     CTcpServer::SendData
///     CTcpServer::GetPort
///     CTcpServer::GetHostname
///     CTcpServer::StartAccept
///     CTcpServer::HandleAccept
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

#include <sstream>
#include <iostream>
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_acceptor.is_open() )
    {
        Logger.Warn << "Closed the TCP server acceptor." << std::endl;
        m_acceptor.close();
    }

    if( m_socket.is_open() )
    {
        Logger.Warn << "Closed an open client connection." << std::endl;
        m_socket.close();
    }
    
    Logger.Status << "Closed TCP server on port " << m_port << std::endl;
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
/// Retrieves a packet of data from the socket delimited with \r\n\r\n.
///
/// @ErrorHandling Throws a std::runtime_error if the socket is not open.
/// @pre The socket must be open, and the data must end with \r\n\r\n.
/// @post Reads data from the socket until the sequence \r\n\r\n.
/// @return A string that contains the entire packet.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string CTcpServer::ReceiveData()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::asio::streambuf packet;
    std::istream packet_stream(&packet);
    std::stringstream sstream;

    if( !m_socket.is_open() )
    {
        throw std::runtime_error("The server has no open client connection.");
    }
    
    Logger.Info << "Blocking to receive data on port " << m_port << std::endl;
    boost::asio::read_until(m_socket, packet, "\r\n\r\n");

    // convert the stream to a string
    sstream << packet_stream.rdbuf();
    return sstream.str();
}

////////////////////////////////////////////////////////////////////////////////
/// Sends a packet of data to the open socket connection.
///
/// @ErrorHandling Throws a std::runtime_error if the socket is not open, or if
/// the packet does not have the correct \r\n\r\n delimiter.
/// @pre The socket must be open, and data must end with \r\n\r\n.
/// @post Sends the passed data over m_socket.
/// @param data The packet of data to send.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CTcpServer::SendData(const std::string data)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::asio::streambuf packet;
    std::ostream packet_stream(&packet);
    
    if( !m_socket.is_open() )
    {
        throw std::runtime_error("The server has no open client connection.");
    }

    if( data.size() < 4 || (data.substr(data.size()-4) != "\r\n\r\n") )
    {
        throw std::runtime_error("The server tried to send a corrupt packet.");
    }
    
    packet_stream << data;
    
    Logger.Info << "Blocking to send data on port " << m_port << std::endl;
    boost::asio::write(m_socket, packet);
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( !m_socket.is_open() )
    {
        throw std::runtime_error("The server has no open client connection.");
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( m_socket.is_open() )
    {
        Logger.Info << "Closed an open client connection." << std::endl;
        m_socket.close();
    }
    m_acceptor.async_accept(m_socket, boost::bind(&CTcpServer::HandleAccept,
            this, boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////
/// Redirects an accepted client to the connection handler.
///
/// @ErrorHandling Throws a std::runtime_error if the connection handler has
/// not been defined with IServer::RegisterHandler.
/// @pre IServer::RegisterHandler must be called prior to this function.
/// @post Calls m_handler to handle the client connection.
/// @post Schedules the next client with CTcpServer::StartAccept.
/// @param error The error code if the connection failed.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CTcpServer::HandleAccept(const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !error )
    {
        Logger.Info << "Accepted a new client connection." << std::endl;
        
        if( m_handler.empty() )
        {
            throw std::runtime_error("The server tried to call a null "
                    + std::string("connection handler."));
        }
        
        m_handler(shared_from_this());
    }
    else
    {
        Logger.Warn << "Failed to accept a client connection." << std::endl;
    }
    if( m_acceptor.is_open() )
    {
        StartAccept();
    }
}

} // namespace device
} // namespace broker
} // namespace freedm

