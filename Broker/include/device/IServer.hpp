////////////////////////////////////////////////////////////////////////////////
/// @file           IServer.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Defines the interface for a server.
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

#ifndef I_SERVER_HPP
#define I_SERVER_HPP

#include <string>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Server interface with variable implementation of the connection handler.
////////////////////////////////////////////////////////////////////////////////
/// Base class for a server with functions for sending and receiving data.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class IServer
    : public boost::enable_shared_from_this<IServer>
    , private boost::noncopyable
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<IServer> Pointer;
    
    /// Type of the callback function for client connections.
    typedef boost::function<void (Pointer)> ConnectionHandler;

    /// Virtual destructor for derived classes.
    virtual ~IServer();
    
    /// Registers a callback function for client connections.
    void RegisterHandler( ConnectionHandler h );
    
    /// Receives a packet of data from the server.
    virtual std::string ReceiveData() const = 0;
    
    /// Sends a packet of data to the connected client.
    virtual void SendData(const std::string pkt) const = 0;
protected:
    /// Callback function to handle clients.
    ConnectionHandler m_handler;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // I_SERVER_HPP
