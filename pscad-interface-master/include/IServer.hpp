///////////////////////////////////////////////////////////////////////////////
/// @file         IServer.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  TCP server that handles a single client at a time
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

#ifndef I_ADAPTER_HPP
#define I_ADAPTER_HPP

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace simulation {

/// single-thread TCP server that handles one client at a time
///////////////////////////////////////////////////////////////////////////////
/// The IServer class handles initialization and teardown of client connections
/// to a TCP server.  Each client is redirected to the pure virtual function
/// IServer::HandleConnection() which must be implemented by derived classes.
/// A derived class can use the member variable m_socket to access the client
/// connection inside of the connection handler.
/// 
/// @limitations The server operates on a single thread and can only handle one
/// client connection at a time.  Additional clients are blocked in a queue
/// until the active connection is fully handled.
///////////////////////////////////////////////////////////////////////////////
class IServer
    : private boost::noncopyable
{
public:
    /// constructs a TCP server on the given port
    IServer( unsigned short port );
    
    /// blocking call to start the server
    void Run();
    /// interrupts and stops the server
    void Stop();
    
    /// closes any active connection
    virtual ~IServer();
private:
    /// prepares to accept the next client
    void StartAccept();
    /// invokes the client connection handler
    void HandleAccept( const boost::system::error_code & error );
    
    /// service for the socket and acceptor
    boost::asio::io_service m_service;
    /// acceptor to receive client connections
    boost::asio::ip::tcp::acceptor m_acceptor;
protected:
    /// handles the accepted socket connection
    virtual void HandleConnection() = 0;
    
    /// socket for accepted client connections
    boost::asio::ip::tcp::socket m_socket;
    /// unique identifier for this server
    unsigned short m_id;
};

} // namespace simulation
} // namespace freedm

#endif // I_ADAPTER_HPP
