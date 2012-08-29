////////////////////////////////////////////////////////////////////////////////
/// @file           ITcpAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>,
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter that communicates
///                 operations over a network.
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

#ifndef ITCPADAPTER_HPP
#define	ITCPADAPTER_HPP

#include "IPhysicalAdapter.hpp"

#include <boost/asio.hpp>

#include <string>

namespace freedm {
namespace broker {
namespace device {

/// Physical adapter interface for network communication applications.
////////////////////////////////////////////////////////////////////////////////
/// @description
///     Physical adapter device interface for network communication 
///     applications. Implementing classes pass Get and Set information to some
///     external source over this interface's socket.
///
/// @limitations    
///     This adapter is designed for connections that require only a single
///     remote peer. If, in the future, a device requires multiple peers, this
///     interface will not be sufficient.
///
/// @todo 
///     Build in some safety so that an error occurs if the network adapter is
///     used before it is connected.
////////////////////////////////////////////////////////////////////////////////
class ITcpAdapter : public IPhysicalAdapter
{
public:
    /// Type of a pointer to a connection adapter.
    typedef boost::shared_ptr<IConnectionAdapter> Pointer;

    /// Creates a socket connection to the given hostname and service.
    virtual void Connect(const std::string hostname, const std::string port);

    /// Virtual destructor for derived classes.
    virtual ~ITcpAdapter() { };

protected:
    /// Constructor to initialize the socket.
    ITcpAdapter(boost::asio::io_service& service);

    /// Closes the connection.
    virtual void Quit() = 0;

    /// Socket to use for the connection.
    mutable boost::asio::ip::tcp::socket m_socket;
};

}
}
}

#endif	// ITCPADAPTER_HPP
