#include "CArmAdapter.hpp"
#include "CAdapterFactory.hpp"
#include "CLogger.hpp"

#include <boost/property_tree/ptree.hpp>

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
    m_HeartbeatPort = p.get<unsigned short>("heartport");
    m_StatePort = p.get<unsigned short>("stateport");
    m_identifier = p.get<std::string>("port");
    
    m_HeartbeatServer = CTcpServer::Create(service, m_HeartbeatPort);
    m_StateServer = CTcpServer::Create(service, m_StatePort);
    
    IServer::ConnectionHandler handler;
    handler = boost::bind(&CArmAdapter::HandleHeartbeat, this, _1);
    m_HeartbeatServer->RegisterHandler(handler);
    handler = boost::bind(&CArmAdapter::HandleState, this, _1);
    m_StateServer->RegisterHandler(handler);
}

CArmAdapter::~CArmAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

void CArmAdapter::Start()
{
    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
}

void CArmAdapter::Send(const std::string data)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::streambuf packet;
    std::ostream packet_stream(&packet);
    packet_stream << data;

    Connect();
    boost::asio::write(m_socket, packet);
    Quit();    
}

void CArmAdapter::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        CAdapterFactory::Instance().RemoveAdapter(m_identifier);
    }
}

void CArmAdapter::Quit()
{
    m_socket.close();
}

void CArmAdapter::HandleHeartbeat(IServer::Pointer connection)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    Heartbeat();
}

void CArmAdapter::HandleState(IServer::Pointer connection)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::stringstream packet, response;
    std::string header;
    
    packet << connection->ReceiveData();
    packet >> header;
    
    if( header == "PoliteDisconnect" )
    {
        response << "PoliteDisconnect: Accepted\r\n\r\n";
    }
    else
    {
        throw std::runtime_error("wat");
    }

    Send(response.str());
}

void CArmAdapter::Heartbeat()
{
    if( m_timer.expires_from_now(boost::posix_time::seconds(5)) != 0 )
    {
        m_timer.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
    }
}

unsigned short CArmAdapter::GetStatePort() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_StatePort;
}

unsigned short CArmAdapter::GetHeartbeatPort() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_HeartbeatPort;
}

} // namespace device
} // namespace broker
} // namespace freedm

