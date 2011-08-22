////////////////////////////////////////////////////////////////////////////////
/// @file           CLineServer.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CLineServer.hpp
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

#include "CLineServer.hpp"

CLineServer::TPointer CLineServer::Create( boost::asio::io_service & p_service, unsigned short p_port, TSetCallback p_set, TGetCallback p_get )
{
    return TPointer( new CLineServer(p_service,p_port,p_set,p_get) );
}

CLineServer::CLineServer( boost::asio::io_service & p_service,
    unsigned short p_port, TSetCallback p_set, TGetCallback p_get )
    : m_acceptor(p_service), m_socket(p_service), m_set(p_set), m_get(p_get)
{
    boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), p_port );
    
    // open the acceptor at the endpoint
    m_acceptor.open( endpoint.protocol() );
    m_acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
    m_acceptor.bind( endpoint );
    m_acceptor.listen();
    
    // wait for connections
    StartAccept();
}

CLineServer::~CLineServer()
{
    // close the socket and acceptor
    if( m_acceptor.is_open() )
    {
        m_acceptor.close();
    }
    if( m_socket.is_open() )
    {
        m_socket.close();
    }
}

void CLineServer::StartAccept()
{
    // wait for next client connection, open it on m_socket, call MessageHandler
    m_acceptor.async_accept( m_socket, boost::bind( &CLineServer::MessageHandler,
        this, boost::asio::placeholders::error ) );
}

void CLineServer::MessageHandler( const boost::system::error_code & p_error )
{
    if( !p_error )
    {
        boost::asio::streambuf request;
        std::istream request_stream( &request );
        std::string request_code, device, key, value;
        
        boost::asio::streambuf response;
        std::ostream response_stream( &response );
        
        bool quit = false;
        
        try
        {
            while( !quit )
            {
                // receive the request stream and get the request type
                boost::asio::read_until( m_socket, request, "\r\n" );
                request_stream >> request_code;
                
                // handle different message types
                if( request_code == "GET" )
                {
                    // split the request stream
                    request_stream >> device >> key;
                    
                    value = m_get(device,key);
                    
                    // format the response stream
                    if( value.empty() )
                    {
                        response_stream << "404 ERROR NOTFOUND\r\n";
                    }
                    else
                    {
                        response_stream << "200 OK " << value << "\r\n";
                    }
                }
                else if( request_code == "SET" )
                {
                    // split the request stream
                    request_stream >> device >> key >> value;
                    
                    m_set(device,key,value);
                    
                    // format the response stream
                    response_stream << "200 OK\r\n";
                }
                else if( request_code == "QUIT" )
                {
                    quit = true;
                    
                    // format the response stream
                    response_stream << "200 OK\r\n";
                }
                else
                {
                    // unrecognized request type
                    response_stream << "400 BADREQUEST\r\n";
                }
                
                // send the response stream
                boost::asio::write( m_socket, response );
            }
        }
        catch( std::exception & e )
        {
            // on error, terminate the connection but continue the server
            std::cerr << "Connection error: " << e.what() << std::endl;
        }
        
        if( m_socket.is_open() )
        {
            // close connection
            m_socket.close();
        }
        
        // wait for next connection
        StartAccept();
    }
}
