#include "CArmAdapter.hpp"
#include "CAdapterFactory.hpp"

namespace freedm {
namespace broker {
namespace device {

IAdapter::Pointer CArmAdapter::Create(boost::asio::io_service & service,
    boost::property_tree::ptree & p)
{
    return CArmAdapter::Pointer(new CArmAdapter(service, p));
}

CArmAdapter::CArmAdapter(boost::asio::io_service & service,
    boost::property_tree::ptree & p)
    : ITcpAdapter(service, p)
    , m_timer(service)
{
    m_port = p.get<unsigned short>("listenport");
    m_server = CTcpServer::Create(service, m_port);
    
    IServer::ConnectionHandler handler;
    handler = boost::bind(&CArmAdapter::HandleConnection, this, _1);
    m_server->RegisterHandler(handler);
}

CArmAdapter::~CArmAdapter()
{
}

void CArmAdapter::Start()
{
    boost::asio::streambuf port;
    std::ostream buffer(&port);

    buffer << m_port << "\r\n";

    IBufferAdapter::Start();

    ITcpAdapter::Connect();
    boost::asio::write(m_socket, port);
    Quit();

    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
}

void CArmAdapter::Timeout(const boost::system::error_code & e)
{
    if( !e )
    {
        CAdapterFactory::Instance().RemoveAdapter(boost::lexical_cast<std::string>(m_port));
    }
}

void CArmAdapter::Quit()
{
    m_socket.close();
}

void CArmAdapter::HandleConnection(IServer::Pointer connection)
{
    m_timer.expires_from_now(boost::posix_time::seconds(5)) > 0;
}

} // namespace device
} // namespace broker
} // namespace freedm

