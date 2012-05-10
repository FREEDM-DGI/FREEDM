////////////////////////////////////////////////////////////////////////////////
/// @file       CPscadAdapter.hpp
///
/// @author     Thomas Roth <tprfh7@mst.edu>
/// @author     Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Client side implementation of the PSCAD simulation line protocol.
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

#ifndef CPSCADADAPTER_HPP
#define CPSCADADAPTER_HPP

#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "IConnectionAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Provides an interface for communicating commands to a PSCAD model
class CPscadAdapter : public IConnectionAdapter
{
    ////////////////////////////////////////////////////////////////////////////////
    /// CLineClient
    ///
    /// @description
    ///     Client side of a line protocol with three requests: GET, SET and QUIT.
    ///     The GET and SET commands operate on (Device,Key) pairs, where Device is
    ///     the unique identifier of some physical hardware and Key is the variable
    ///     name of that hardware to be manipulated.
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////////
public:
    typedef boost::shared_ptr<CPscadAdapter> AdapterPointer;
    static AdapterPointer Create(boost::asio::io_service & service);

    ////////////////////////////////////////////////////////////////////////////
    /// Set( const string &, const string &, const string & )
    ///
    /// @description
    ///     Sends a set request to the line server with a desired set value.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     Throws an exception if the server does not acknowledge the request.
    ///
    /// @pre
    ///     The socket connection has been established with a call to Connect
    ///
    /// @post
    ///     Writes a set message to m_socket
    ///     Reads an acknowledgment from m_socket
    ///
    /// @param
    ///     device is the unique identifier of the target device
    ///     key is the variable of the target device to modify
    ///     value is the value to set for device's key
    ///
    /// @limitations
    ///     The precondition is not enforced.
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Set(const Identifier device, const SettingKey key,
            const SettingValue value);

    ////////////////////////////////////////////////////////////////////////////
    /// Get( const string &, const string & )
    ///
    /// @description
    ///     Sends a get request to the line server and returns the response.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     Throws an exception if the server does not respond to the request.
    ///
    /// @pre
    ///     The socket connection has been established with a call to Connect
    ///
    /// @post
    ///     Writes a get message to m_socket
    ///     Reads a response from m_socket
    ///
    /// @param
    ///     device is the unique identifier of the target device
    ///     key is the variable of the target device to access
    ///
    /// @return
    ///     device's key as determined by the line server response
    ///
    /// @limitations
    ///     The precondition is not enforced.
    ///
    ////////////////////////////////////////////////////////////////////////////
    SettingValue Get(const Identifier device, const SettingKey key) const;

    ////////////////////////////////////////////////////////////////////////////
    /// Quit
    ///
    /// @description
    ///     Sends a quit request to the line server and closes the socket.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     Throws an exception if the server does not acknowledge the request.
    ///
    /// @pre
    ///     The socket connection has been established with a call to Connect
    ///
    /// @post
    ///     Writes a quit message to m_socket
    ///     Reads an acknowledgement from m_socket
    ///     Closes m_socket if no exception occurs
    ///
    /// @limitations
    ///     The precondition is not enforced.
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Quit();

    ////////////////////////////////////////////////////////////////////////////
    /// ~CLineClient
    ///
    /// @description
    ///     Closes the socket before destroying an object instance.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     m_socket is closed
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////    
    ~CPscadAdapter();
private:
    ////////////////////////////////////////////////////////////////////////////
    /// CLineClient( io_service & )
    ///
    /// @description
    ///     Creates a line protocol client on the given service.
    ///
    /// @Shared_Memory
    ///     Uses the passed io_service until destroyed.
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     none
    ///
    /// @post
    ///     io_service is shared with m_socket
    ///
    /// @param
    ///     service is the io_service the socket runs on
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    CPscadAdapter(boost::asio::io_service & service);
};

}//namespace broker
}//namespace freedm
}//namespace device

#endif // CPSCADADAPTER_HPP
