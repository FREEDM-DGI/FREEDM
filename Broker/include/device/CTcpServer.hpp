////////////////////////////////////////////////////////////////////////////////
/// @file           CTcpServer.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    TCP server that accepts a single connection at a time.
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

#ifndef C_TCP_SERVER_HPP
#define C_TCP_SERVER_HPP

#include "IServer.hpp"

#include <boost/asio.hpp>

namespace freedm {
namespace broker {
namespace device {

/// TCP server that handles a single client connection.
////////////////////////////////////////////////////////////////////////////////
/// A TCP server that redirects clients to the registered connection handler
/// and handles packets that are delimited with the sequence \r\n\r\n.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CTcpServer
    : public IServer
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<CTcpServer> Pointer;

    /// Creates a new TCP server on the specified port number.
    static Pointer Create(boost::asio::io_service & ios, unsigned short port);

    /// Stops the TCP server.
    virtual ~CTcpServer();
    
    /// Receives a packet from the connected client.
    std::string ReceiveData() const;
    
    /// Sends a packet of data to the connected client.
    void SendData(const std::string pkt) const;
    
    /// Gets the listen port of the TCP server.
    unsigned short GetPort() const;
    
    /// Gets the hostname of the connected client.
    std::string GetHostname() const;
private:
    /// Constructs the TCP server on the specified port number.
    CTcpServer(boost::asio::io_service & ios, unsigned short port);
    
    /// Prepares to accept the next client.
    void StartAccept();
    
    /// Handles an accepted client connection.
    void HandleAccept(const boost::system::error_code & error);
    
    /// Acceptor for new client connections.
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    /// Socket for the current client.
    mutable boost::asio::ip::tcp::socket m_socket;
    
    /// Port number of the server.
    unsigned short m_port;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_TCP_SERVER_HPP

