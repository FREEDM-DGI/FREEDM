////////////////////////////////////////////////////////////////////////////////
/// @file           IConnectionAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>,
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Device adapter that communicates operations over a network.
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

#ifndef I_CONNECTION_ADAPTER_HPP
#define	I_CONNECTION_ADAPTER_HPP

#include "IPhysicalAdapter.hpp"

#include <string>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Physical adapter interface for network communication applications.
////////////////////////////////////////////////////////////////////////////////
/// Physical device adapter for TCP network communications.  This base class
/// contains a socket that can be used to implement a communication protocol.
///
/// @limitations The adapter can communicate with at most one remote peer.  If
/// a device needs to communicate with multiple peers, then this class is not
/// sufficient.
////////////////////////////////////////////////////////////////////////////////
class IConnectionAdapter
    : public IPhysicalAdapter
{
public:
    /// Type of a shared pointer to a connection adapter.
    typedef boost::shared_ptr<IConnectionAdapter> Pointer;

    /// Creates a socket connection to the given hostname and port number.
    void Connect(std::string hostname, std::string port);

    /// Closes the connection.
    virtual void Quit() = 0;

    /// Virtual destructor for derived classes.
    virtual ~IConnectionAdapter();
protected:
    /// Constructor to initialize the socket.
    IConnectionAdapter(boost::asio::io_service & service);

    /// Socket to use for the TCP connection.
    mutable boost::asio::ip::tcp::socket m_socket;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif	// I_CONNECTION_ADAPTER_HPP
