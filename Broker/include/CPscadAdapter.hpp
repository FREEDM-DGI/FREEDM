////////////////////////////////////////////////////////////////////////////////
/// @file           CPscadAdapter.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description
///     Client side implementation of the simulation line protocol.
///
/// @functions
///     CLineClient::Create( io_service & )
///     CLineClient::Connect( const string &, const string & )
///     CLineClient::Set( const string &, const string &, const string & )
///     CLineClient::Get( const string &, const string & )
///     CLineClient::Quit()
///
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPSCADADAPTER_HPP
#define CPSCADADAPTER_HPP

#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm{
  namespace broker{
/// Provides an interface for communicating commands to a PSCAD model
class CPscadAdapter : private boost::noncopyable
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
    typedef boost::shared_ptr<CPscadAdapter> TPointer;
    static TPointer Create( boost::asio::io_service & p_service );
    
    ////////////////////////////////////////////////////////////////////////////
    /// Connect( const string &, const string & )
    ///
    /// @description
    ///     Creates a socket connection to the given hostname and service.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     Throws an exception for unexpected connection errors.
    ///
    /// @pre
    ///     p_hostname and p_service specify a valid endpoint
    ///
    /// @post
    ///     m_socket attempts to connect to the passed service
    ///
    /// @param
    ///     p_hostname is the hostname of the desired endpoint
    ///     p_port is the port number of the desired endpoint
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Connect( const std::string p_hostname, const std::string p_port );
    
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
    ///     Reads an acknowledgement from m_socket
    ///
    /// @param
    ///     p_device is the unique identifier of the target device
    ///     p_key is the variable of the target device to modify
    ///     p_value is the value to set for p_device's p_key
    ///
    /// @limitations
    ///     The precondition is not enforced.
    ///
    ////////////////////////////////////////////////////////////////////////////
    void Set( const std::string p_device, const std::string p_key, const std::string p_value );
    
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
    ///     p_device is the unique identifier of the target device
    ///     p_key is the variable of the target device to access
    ///
    /// @return
    ///     p_device's p_key as determined by the line server response
    ///
    /// @limitations
    ///     The precondition is not enforced.
    ///
    ////////////////////////////////////////////////////////////////////////////
    std::string Get( const std::string p_device, const std::string p_key );
    
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
    ///     p_service is the io_service the socket runs on
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    CPscadAdapter( boost::asio::io_service & p_service );
    
    /// socket to line protocol server
    boost::asio::ip::tcp::socket m_socket;
};

  }//namespace broker
}//namespace freedm

#endif // CPSCADADAPTER_HPP
