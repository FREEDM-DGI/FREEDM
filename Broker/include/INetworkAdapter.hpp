////////////////////////////////////////////////////////////////////////////////
/// @file           INetworkAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>,
///                 Yaxi Liu <ylztf@mst.edu>,
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Interface for a physical device adapter that communicates
///                 operations over a network.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef IPHYSICALADAPTER_HPP
#define	IPHYSICALADAPTER_HPP

#include <boost/asio.hpp>

#include <string>

////////////////////////////////////////////////////////////////////////////////
/// @brief  Physical adapter device interface.
///
/// @description
///     Physical adapter device interface. Each device contains a reference to
///     an adapter that it uses to perform all operations. The adapter is
///     responsible for implementing the behavior of "get value" and "set value" 
///     operations on devices. The adapter is, in effect, the device's "driver".
///     Note that the same adapter can be used for all devices in the
///     simulation if this is desirable.
///
/// @limitations    
///     None.
////////////////////////////////////////////////////////////////////////////////
class INetworkAdapter //: public IPhysicalAdapter
{
public:
    /// Creates a socket connection to the given hostname and service.
    virtual void Connect(const std::string hostname, const std::string port);

    /// Closes the connection.
    virtual void Quit() = 0;

protected:
    /// Constructor to initialize the socket.
    INetworkAdapter(boost::asio::io_service& service);
    
    /// Socket to use for the connection.
    boost::asio::ip::tcp::socket m_socket;
};

#endif	// IPHYSICALADAPTER_HPP
