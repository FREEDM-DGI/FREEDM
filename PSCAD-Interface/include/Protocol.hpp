#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::tcp;

#ifndef __PRETTY_FUNCTION__
    #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

//#ifndef Debug
//    #define Debug std::cerr
//#endif

//namespace freedm{

// IProtocol implements
class IProtocol

{
public:
	tcp::socket& GetSocket()
	{
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
		return m_socket;
	}

protected:
	tcp::socket	m_socket;

	IProtocol( boost::asio::io_service & p_service ) :
		m_socket( p_service )
	{
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
	};

};


class IClientProtocol : public IProtocol
{
public:
	virtual void HandleConnect( const boost::system::error_code &p_error ) = 0;
protected:
	IClientProtocol( boost::asio::io_service & p_service ) :
		IProtocol( p_service )
	{
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
	};
};

class IServerProtocol: public IProtocol
{
public:
	virtual void HandleAccept( const boost::system::error_code & ) = 0;
protected:
	explicit IServerProtocol( boost::asio::io_service & p_service ) :
		IProtocol( p_service )
	{
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
	};
};

//}
