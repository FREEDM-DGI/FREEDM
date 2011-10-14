////////////////////////////////////////////////////////////////////////////////
/// @file           CLineServer.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @description
///     Server side implementation of the simulation line protocol.
///
/// @functions
///     Create( io_service &, unsigned short, TGetCallback, TSetCallback )
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
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef C_LINE_SERVER_HPP
#define C_LINE_SERVER_HPP

#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "logger.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace simulation {

////////////////////////////////////////////////////////////////////////////////
/// CLineServer
///
/// @description
///     Server side of a line protocol with three requests: GET, SET and QUIT.
///     The GET and SET commands operate on (Device,Key) pairs, where Device is
///     the unique identifier of some physical hardware and Key is the variable
///     name of that hardware to be manipulated.
///
/// @limitations
///     none
///
////////////////////////////////////////////////////////////////////////////////
class CLineServer : private boost::noncopyable
{
public:
    typedef boost::function< void ( const std::string &, const std::string &, const std::string & ) > TSetCallback;
    typedef boost::function< std::string ( const std::string &, const std::string & ) > TGetCallback;
    typedef boost::shared_ptr<CLineServer> TPointer;
    
    static TPointer Create( boost::asio::io_service & p_service, unsigned short p_port, TSetCallback p_set, TGetCallback p_get );
    
    ////////////////////////////////////////////////////////////////////////////
    /// ~CLineServer
    ///
    /// @description
    ///     Closes the acceptor before destroying an object instance.
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
    ///     m_acceptor is closed
    ///     m_socket is closed
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    ~CLineServer();
private:
    ////////////////////////////////////////////////////////////////////////////
    /// CLineServer( io_service &, unsigned short, TGetCallback, TSetCallback )
    ///
    /// @description
    ///     Creates a line protocol server using the given callback functions.
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
    ///     io_service is shared with m_acceptor and m_socket
    ///     connections to m_acceptor are redirected to MessageHandler
    ///
    /// @param
    ///     p_service is the io_service the acceptor and socket run on
    ///     p_port is the port the acceptor listens on
    ///     p_set is the function called for SET requests
    ///     p_get is the function called for GET requests
    ///
    /// @limitations
    ///     p_set : void ( const string &, const string &, const string & )
    ///     p_get : string ( const string &, const string & )
    ///
    ////////////////////////////////////////////////////////////////////////////
    CLineServer( boost::asio::io_service & p_service, unsigned short p_port, TSetCallback p_set, TGetCallback p_get );
    
    ////////////////////////////////////////////////////////////////////////////
    /// StartAccept
    ///
    /// @description
    ///     Waits for a client connection and redirects it to MessageHandler.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     none
    ///
    /// @pre
    ///     m_socket is closed
    ///
    /// @post
    ///     m_socket set to the new client connection
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    void StartAccept();
    
    ////////////////////////////////////////////////////////////////////////////
    /// MessageHandler
    ///
    /// @description
    ///     Handles all messages from a client until QUIT is received.
    ///
    /// @Shared_Memory
    ///     none
    ///
    /// @Error_Handling
    ///     Terminates the client connection if an exception occurs.
    ///
    /// @pre
    ///     m_socket is an open connection
    ///
    /// @post
    ///     read and write operations occur over m_socket
    ///     m_socket is closed when the function returns
    ///
    /// @param
    ///     p_error is an error code set on accept failure
    ///
    /// @limitations
    ///     none
    ///
    ////////////////////////////////////////////////////////////////////////////
    void MessageHandler( const boost::system::error_code & p_error );
    
    /// entry queue for client connections
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    /// socket to line procotol client
    boost::asio::ip::tcp::socket m_socket;
    
    /// set callback function
    TSetCallback m_set;
    
    /// get callback function
    TGetCallback m_get;
    
    /// port number for debug output
    unsigned short m_port;
};

} // namespace simulation
} // namespace freedm

#endif // C_LINE_SERVER_HPP
