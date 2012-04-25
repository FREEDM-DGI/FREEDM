////////////////////////////////////////////////////////////////////////////////
/// @file           CLineClient.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CLineClient.hpp
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

#include "CLineClient.hpp"

namespace freedm {

namespace broker {

CLineClient::TPointer CLineClient::Create( boost::asio::io_service & p_service )
{
  return CLineClient::TPointer( new CLineClient(p_service) );
}

CLineClient::CLineClient( boost::asio::io_service & p_service )
    : m_socket(p_service)
{
    // skip
}

void CLineClient::Connect( const std::string p_hostname, 
        const std::string p_port )
{
    boost::asio::ip::tcp::resolver resolver( m_socket.get_io_service() );
    boost::asio::ip::tcp::resolver::query query( p_hostname, p_port );
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    
    // attempt to connect to one of the resolved endpoints
    boost::system::error_code error = boost::asio::error::host_not_found;
    while( error && it != end )
    {
        m_socket.close();
        m_socket.connect( *it, error );
        ++it;
    }
    if( error )
    {
        std::stringstream ss;
        ss << "CLineClient attempted to connect to " << p_hostname << " on port"
                << " " << p_port << ", but connection failed for the following "
                << "reason: " << boost::system::system_error(error).what();
        throw std::runtime_error(ss.str());
    }
    
    return( it != end );
}

void CLineClient::Set( const std::string p_device, const std::string p_key,
    const std::string p_value )
{
    boost::asio::streambuf request;
    std::ostream request_stream( &request );
    
    boost::asio::streambuf response;
    std::istream response_stream( &response );
    std::string response_code, response_message;
    
    // format and send the request stream
    request_stream << "SET " << p_device << ' ' << p_key << ' ' << p_value 
            << "\r\n";
    boost::asio::write( m_socket, request );
    
    // receive and split the response stream
    boost::asio::read_until( m_socket, response, "\r\n" );
    response_stream >> response_code >> response_message;
    
    // handle bad responses
    if( response_code != "200" )
    {
        std::stringstream ss;
        ss << "CLineClient attempted to set " << p_key << " to " << p_value
                << " on device " << p_device << ", but received a PSCAD error: "
                << response_message;
        throw std::runtime_error(ss.str());
    }
}

std::string CLineClient::Get( const std::string p_device, 
        const std::string p_key )
{
    boost::asio::streambuf request;
    std::ostream request_stream( &request );
    
    boost::asio::streambuf response;
    std::istream response_stream( &response );
    std::string response_code, response_message, value;
    
    // format and send the request stream
    request_stream << "GET " << p_device << ' ' << p_key << "\r\n";
    boost::asio::write( m_socket, request );
    
    // receive and split the response stream
    boost::asio::read_until( m_socket, response, "\r\n" );
    response_stream >> response_code >> response_message >> value;
    
    // handle bad responses
    if( response_code != "200" )
    {
        std::stringstream ss;
        ss << "CLineClient attempted to get " << p_key << " on device " 
                << p_device << ", but received a PSCAD error: "
                << response_message;
        throw std::runtime_error(ss.str());
    }
    
    return value;
}

void CLineClient::Quit()
{
    boost::asio::streambuf request;
    std::ostream request_stream( &request );
    
    boost::asio::streambuf response;
    std::istream response_stream( &response );
    std::string response_code, response_message;
    
    // format and send the request stream
    request_stream << "QUIT\r\n";
    boost::asio::write( m_socket, request );
    
    // receive and split the response stream
    boost::asio::read_until( m_socket, response, "\r\n" );
    response_stream >> response_code >> response_message;
    
    // handle bad responses
    if( response_code != "200" )
    {
        std::stringstream ss;
        ss << "CLineClient attempted quit, but received a PSCAD error: "
                << response_message;
        throw std::runtime_error(ss.str());
    }
    
    // close connection
    m_socket.close();
}

CLineClient::~CLineClient()
{
    //  perform teardown
    if( m_socket.is_open() )
    {
        Quit();
    }
}

  }//namespace broker
}//namespace freedm
