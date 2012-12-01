#include "CTcpServer.hpp"
#include "CLogger.hpp"

#include <boost/bind.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace // unnamed
{
    CLocalLogger Logger(__FILE__);
}

CTcpServer::Pointer CTcpServer::Create( boost::asio::io_service & ios,
        unsigned short port )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return Pointer(new CTcpServer(ios, port));
}

CTcpServer::CTcpServer( boost::asio::io_service & ios, unsigned short port )
    : m_acceptor(ios)
    , m_socket(ios)
    , m_id(port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), port);

    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();

    Logger.Status << "Opened TCP server on port " << port << "." << std::endl;

    StartAccept();
}

CTcpServer::~CTcpServer()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_acceptor.is_open() )
    {
        Logger.Warn << "Closed the TCP server acceptor." << std::endl;
        m_acceptor.close();
    }

    if( m_socket.is_open() )
    {
        Logger.Warn << "Closed an open client connection." << std::endl;
        m_socket.close();
    }
}

std::string CTcpServer::ReceiveData()
{
    boost::asio::streambuf packet;
    std::istream packet_stream(&packet);
    std::stringstream sstream;

    boost::asio::read_until(m_socket, packet, "\r\n");

    sstream << packet_stream.rdbuf();
    return sstream.str();
}

void CTcpServer::SendData(const std::string str)
{
    boost::asio::streambuf packet;
    std::ostream packet_stream(&packet);
    
    packet_stream << str << "\r\n";
    boost::asio::write(m_socket, packet);
}

void CTcpServer::StartAccept()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( m_socket.is_open() )
    {
        Logger.Info << "Closed an open client connection." << std::endl;
        m_socket.close();
    }
    m_acceptor.async_accept(m_socket, boost::bind(&CTcpServer::HandleAccept,
        this, boost::asio::placeholders::error));
}

std::string CTcpServer::GetHostname() const
{
    return m_socket.remote_endpoint().address().to_string();
}

void CTcpServer::HandleAccept( const boost::system::error_code & error )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !error )
    {
        Logger.Info << "Accepted a new client connection." << std::endl;
        m_handler(shared_from_this());
    }
    else
    {
        Logger.Warn << "Failed to accept a client connection." << std::endl;
    }
    StartAccept();
}

} // namespace device
} // namespace broker
} // namespace freedm

