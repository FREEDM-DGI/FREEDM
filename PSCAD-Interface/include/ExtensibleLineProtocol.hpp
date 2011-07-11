#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Protocol.hpp"

#ifndef __PRETTY_FUNCTION__
    #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

//#ifndef Debug
//  #define Debug std::cerr
//#endif

namespace freedm {
class CExtensibleLineClient : public IClientProtocol
{
public:
	typedef boost::shared_ptr< CExtensibleLineClient > Pointer;
	static Pointer Create( boost::asio::io_service &p_service )
	{
		return Pointer( new CExtensibleLineClient( p_service ));
	};

	virtual ~CExtensibleLineClient()
	{
		if( true == m_socket.is_open() )
		{
			Quit();
		}
	}

	void Connect( std::string p_hostname, std::string p_service )
	{
		tcp::resolver resolver( m_socket.get_io_service() );
		tcp::resolver::query query( p_hostname, p_service );
		tcp::resolver::iterator endpoint_iterator = resolver.resolve( query );
		tcp::resolver::iterator end;

		// Default the error to HOST NOT FOUND if the iterator list is empty
		boost::system::error_code error = boost::asio::error::host_not_found;
		while( error && (endpoint_iterator != end ) )
		{
			m_socket.close();
			m_socket.connect( *endpoint_iterator, error );

			endpoint_iterator++;
		}

		if( error )
		{
			throw boost::system::system_error( error );
		}

		return;
	};

	virtual void HandleConnect( const boost::system::error_code &p_error )
	{
	    // We could use this to create an asynchronous communication script,
	    // but for this particular client protocol it makes sense to make it
	    // synchronous
	};

	void Set( const std::string &p_key, const std::string &p_value )
	{
		std::string response_code;
		boost::asio::streambuf request;
		std::ostream request_stream( &request );

		boost::asio::streambuf response;
		std::istream response_stream( &response );

		// NOTE: The write() and read_until() operations can throw exceptions
		// Both throw boost::system::system_error; let it propogate for now
		request_stream << "SET " << p_key  << ' ' <<  p_value << "\r\n";
		boost::asio::write( m_socket, request );

		boost::asio::read_until( m_socket, response, "\r\n");
		response_stream >> response_code;

		if( "200" != response_code )
		{
            std::cerr << "Error: " << response_code;
		    response_stream >> response_code;
		    std::cerr << "\t" << response_code << std::endl;
		}
	};

	std::string Get( const std::string &p_key )
	{
		std::string response_code;

		boost::asio::streambuf request;
		std::ostream request_stream( &request );

		boost::asio::streambuf response;
		std::istream response_stream( &response );

		// NOTE: The write() and read_until() operations can throw exceptions
		// Both throw boost::system::system_error; let it propogate for now
		request_stream << "GET " << p_key << "\r\n";
		boost::asio::write( m_socket, request );

		boost::asio::read_until( m_socket, response, "\r\n");
		response_stream >> response_code;

		if( "200" == response_code )
		{
		    // Should be OK
		    response_stream >> response_code;

		    // Should be value
            response_stream >> response_code;
		}
        else
        {
            response_stream >> response_code;
            std::cerr << response_code << ": ";
            response_stream >> response_code;
        }

		return response_code;

	};

	void Quit()
	{
		// Send QUIT
		std::string response_code;

		boost::asio::streambuf request;
		std::ostream request_stream( &request );

		boost::asio::streambuf response;
		std::istream response_stream( &response );

		for( int i = 0; i < 5; ++i)
		{
			// We will attempt to tell the server that we are QUIT-ing
			// up to 5 times.  If the server still fails, we'll disconnect.
			// Hopefully it will eventually figure it out.

			// NOTE: The write() and read_until() operations can throw exceptions
			// Both throw boost::system::system_error; let it propogate for now
			request_stream << "QUIT " << "\r\n";
			boost::asio::write( m_socket, request );

			boost::asio::read_until( m_socket, response, "\r\n");
			response_stream >> response_code;

			if( "200" == response_code )
			{
				// Server acknoledges QUIT
				break;
			}
		} // End for i

		// Close the socket.
		m_socket.close();
	};

private:
	CExtensibleLineClient( boost::asio::io_service& p_service ) :
		IClientProtocol( p_service )
	{ };

};



class CExtensibleLineServer :
    public IServerProtocol,
    public boost::enable_shared_from_this< CExtensibleLineServer >
{
public:
    typedef boost::shared_ptr< CExtensibleLineServer > Pointer;
    typedef boost::function< const std::string ( const std::string & ) > GetCallback;
    typedef boost::function< void ( const std::string &, const std::string & ) > SetCallback;

    static Pointer Create( boost::asio::io_service& p_service,
        GetCallback p_getCB,
        SetCallback p_setCB )
    {
        std::cerr << __PRETTY_FUNCTION__ << std::endl;

        return Pointer( new CExtensibleLineServer( p_service, p_getCB, p_setCB ));
    };

	virtual void HandleAccept( const boost::system::error_code &p_error )
	{
        std::cerr << __PRETTY_FUNCTION__ << std::endl;

	    HandleRead( p_error );
	};

private:
    explicit CExtensibleLineServer( boost::asio::io_service& p_service,
        GetCallback p_getCB,
        SetCallback p_setCB ) :
            IServerProtocol( p_service ),
            m_getValueCallback( p_getCB ),
            m_setValueCallback( p_setCB )
    {
        std::cerr << __PRETTY_FUNCTION__ << std::endl;
    };

    void HandleRead( const boost::system::error_code &p_error )
    {
        std::cerr << __PRETTY_FUNCTION__ << std::endl;

        if( !p_error )
        {
            std::string request_string, request_key, request_value;
            bool quit, success;
            std::size_t bytes_read;

            boost::asio::streambuf request;
            std::istream request_stream( &request );

            boost::asio::streambuf response;
            std::ostream response_stream( &response );

            quit = false;
            try {
                do {
                    bytes_read = boost::asio::read_until( m_socket, request, "\r\n");
                    request_stream >> request_string;

                    if( "GET" == request_string )
                    {
                        request_stream >> request_key;
                        request_value = m_getValueCallback( request_key );

                        if( "NOTFOUND" == request_value )
                        {
                            response_stream << "404 ERROR\r\n";
                        }
                        else
                        {
                            response_stream << "200 OK" << "\r\n";
                        }

                        response_stream << request_value  << "\r\n";
                    }
                    else if ( "SET" == request_string )
                    {
                        request_stream >> request_key;
                        request_stream >> request_value;

                        m_setValueCallback( request_key, request_value );

                        response_stream << "200 OK" << "\r\n";
                    }
                    else if ( "QUIT" == request_string )
                    {
                        response_stream << "200 OK - GOODBYE" << "\r\n";
                        quit = true;
                    }
                    else
                    {
                        // Unrecogized command
                        response_stream << "400 BADREQUEST" << "\r\n";
                    }

                    // Send the response
                    boost::asio::write( m_socket, response );
                    request.consume( bytes_read );
                } while ( false == quit );
            } catch ( std::exception &e )
            {
                std::cerr << "Connection unexpectedly quit." << std::endl
                	<< "\t" << e.what() << std::endl;
            }

            std::cerr << "Closing the session" << std::endl;
            m_socket.close();
        }
        else if ( p_error != boost::asio::error::operation_aborted )
        {
            std::cerr << "Read error" << std::endl;
        }
    };

    boost::array< char, 8192>   m_buffer;
    GetCallback                 m_getValueCallback;
    SetCallback                 m_setValueCallback;

};

class CExtensibleService
{
public:
    CExtensibleService( boost::asio::io_service &p_service, const int p_port,
        CExtensibleLineServer::GetCallback p_getCB, CExtensibleLineServer::SetCallback p_setCB ) :
            m_acceptor( p_service, tcp::endpoint( tcp::v4(), p_port) ),
            m_getCB( p_getCB ),
            m_setCB( p_setCB )
    {
        std::cerr << __PRETTY_FUNCTION__ << std::endl;
        StartAccept( );
    };

protected:
    void StartAccept( )
    {
        std::cerr << __PRETTY_FUNCTION__ << std::endl;
        CExtensibleLineServer::Pointer new_session =
            CExtensibleLineServer::Create( m_acceptor.get_io_service(), m_getCB, m_setCB );

        m_acceptor.async_accept( new_session->GetSocket(),
            boost::bind( &CExtensibleService::HandleAccept, this, new_session,
                boost::asio::placeholders::error ));
    };

    void HandleAccept( CExtensibleLineServer::Pointer p_session,
        const boost::system::error_code &p_error )
    {
        std::cerr << __PRETTY_FUNCTION__ << std::endl;

        if( !p_error )
        {
            p_session->HandleAccept( p_error );
            StartAccept( );
        }
        else
        {
            std::cerr << "HandleAccept: error " <<
                boost::system::system_error(p_error).what() << std::endl;
        }
    };

private:
    tcp::acceptor m_acceptor;

    CExtensibleLineServer::GetCallback m_getCB;
    CExtensibleLineServer::SetCallback m_setCB;
};
}
