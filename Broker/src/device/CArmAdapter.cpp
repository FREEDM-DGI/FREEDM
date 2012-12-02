#include "CArmAdapter.hpp"
#include "CAdapterFactory.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
    CLocalLogger Logger(__FILE__);
}

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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

void CArmAdapter::Start()
{
    boost::asio::streambuf port;
    std::ostream buffer(&port);

    buffer << m_port;

    IBufferAdapter::Start();

    ITcpAdapter::Connect();
    boost::asio::write(m_socket, port);
    Quit();

    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
}

void CArmAdapter::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        Logger.Debug << "reset" << std::endl;
        CAdapterFactory::Instance().RemoveAdapter(boost::lexical_cast<std::string>(m_port));
    }
}

void CArmAdapter::Quit()
{
    m_socket.close();
}

void CArmAdapter::HandleConnection(IServer::Pointer connection)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if( m_timer.expires_from_now(boost::posix_time::seconds(5)) == 0 )
        Logger.Warn << "noooooo" << std::endl;
    else
        m_timer.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
}

} // namespace device
} // namespace broker
} // namespace freedm

