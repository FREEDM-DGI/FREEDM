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
///     CTcpServer::Stop
///     CTcpServer::GetPort
///     CTcpServer::GetSocket
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
/// @param address The address of the interface used for listening.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CTcpServer::CTcpServer(boost::asio::io_service & ios, unsigned short port,
    const std::string address)
    : m_acceptor(ios)
    , m_port(port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    using boost::asio::ip::tcp;

    tcp::endpoint endpoint(tcp::v4(), port);

    if( address != "" )
    {
        endpoint.address( boost::asio::ip::address::from_string(address) );
    }

    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();

    Logger.Status << "Opened TCP server: " << endpoint << "." << std::endl;

    StartAccept();
}

////////////////////////////////////////////////////////////////////////////////
/// Stops the server prior to destruction.
///
/// @pre None.
/// @post Calls CTcpServer::Stop.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CTcpServer::~CTcpServer()
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;
    Stop();
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new TCP server.
///
/// @pre The specified port number must be valid.
/// @post Creates a TCP server that accepts connections on the given port.
/// @param ios The I/O service used by the server.
/// @param port The listen port of the server.
/// @param address The address of the interface used for listening.
/// @return Shared pointer to the server.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CTcpServer::Pointer CTcpServer::Create(boost::asio::io_service & ios,
        unsigned short port, const std::string address)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Pointer(new CTcpServer(ios, port, address));
}

////////////////////////////////////////////////////////////////////////////////
/// Stops the TCP server from accepting new clients.
///
/// @pre None.
/// @post m_acceptor and m_socket are closed.
/// @ErrorHandling throws boost::system::system_error if it fails
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CTcpServer::Stop()
{
    Logger.Trace << hdr() << __PRETTY_FUNCTION__ << std::endl;

    if( m_acceptor.is_open() )
    {
        Logger.Info << hdr() << "Closed TCP server acceptor." << std::endl;
        m_acceptor.close();
    }

    Logger.Status << "Closed TCP server on port " << m_port << "." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Registers a client connection handler with the server.
///
/// @ErrorHandling Throws a std::runtime_error if either m_handler has already
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

    if( m_acceptor.is_open() )
    {
        m_client.reset(new boost::asio::ip::tcp::socket(m_acceptor.get_io_service()));
        m_acceptor.async_accept(*m_client, boost::bind(&CTcpServer::HandleAccept,
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
/// @param error The error code if the connection failed.
///
/// @limitations This function will not schedule the next accept. The owner of
/// the handler must call CTcpServer::StartAccept when done with the client.
/// This limitation is because the server handles at most one connection, and
/// that connection must be closed before the next accept can be scheduled.
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
    else if( error != boost::asio::error::operation_aborted )
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

} // namespace device
} // namespace broker
} // namespace freedm

