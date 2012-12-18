#include "CArmAdapter.hpp"
#include "CAdapterFactory.hpp"
#include "CLogger.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
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
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CArmAdapter::Pointer(new CArmAdapter(service, p));
}

CArmAdapter::CArmAdapter(boost::asio::io_service & service,
        boost::property_tree::ptree & p)
    : m_heartbeat(service)
    , m_command(service)
    , m_initialized(0)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_identifier = p.get<std::string>("identifier");
    m_port = p.get<unsigned short>("stateport");
    
    IServer::ConnectionHandler handler;
    handler = boost::bind(&CArmAdapter::HandleMessage, this, _1);
    m_server = CTcpServer::Create(service, m_port);
    m_server->RegisterHandler(handler);
}

CArmAdapter::~CArmAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

void CArmAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_heartbeat.expires_from_now(boost::posix_time::seconds(5));
    m_heartbeat.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
}

void CArmAdapter::Heartbeat()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_heartbeat.expires_from_now(boost::posix_time::seconds(5)) != 0 )
    {
        m_heartbeat.async_wait(boost::bind(&CArmAdapter::Timeout, this, _1));
    }
}

unsigned short CArmAdapter::GetStatePort() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_port;
}

void CArmAdapter::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        CAdapterFactory::Instance().RemoveAdapter(m_identifier);
    }
}

void CArmAdapter::HandleMessage(IServer::Pointer connection)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::stringstream packet, response;
    std::string header, devname, signal, value;
    std::size_t index;
    
    try
    {
        packet << connection->ReceiveData();
        Heartbeat();
    }
    catch(std::exception & e)
    {
        return;
    }
    
    packet >> header
    
    if( header == "DeviceStates" )
    {
        boost::unique_lock<boost::shared_mutex> lock(m_rxMutex);
        
        while( packet >> devname >> signal >> value )
        {
            devname = m_identifier + ":" + devname;
            
            if( m_stateInfo.count(devname) == 0 )
            {
                throw std::runtime_error("oops");
            }
            
            index = m_stateInfo[devname];
            m_rxBuffer[index] = boost::lexical_cast<DeviceSignal>(value);
        }
        
        if( !m_initialized )
        {
            m_initialized = true;
            m_command.expires_from_now(boost::posix_time::seconds(2));
            m_command.async_wait(boost::bind(&CArmAdapter::SendCommandPacket, this, _1));
        }
    }
    else if( header == "PoliteDisconnect" )
    {
        response << "PoliteDisconnect: Accepted\r\n\r\n";
    }
    else
    {
        throw std::runtime_error("bad");
    }
    
    try
    {
        connection->SendData(response.str());
        Heartbeat();
    }
    catch(std::exception & e)
    {
        return;
    }
}

void CArmAdapter::SendCommandPacket()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::map<const DeviceSignal, const std::size_t>::iterator it, end;
    std::stringstream packet;
    std::string devname, signal;
    SignalValue value;
    std::size_t index;
    
    boost::unique_lock<boost::mutex> lock(m_txMutex);
    
    end = m_commandInfo.end();
    for( it = m_commandInfo.begin(); it != end; it++ )
    {
        devname = it->first.first;
        signal = it->first.second;
        
        // remove the hostname identifier
        index = devname.find_last_of(":");
        devname = devname.substr(index+1);
        
        value = m_txBuffer[it->second];
        
        packet << devname << " " << signal << " " << value << "\r\n";
    }
    packet << "\r\n";
    
    try
    {
        connection->SendData(packet.str());
        Heartbeat();
    }
    catch(std::exception & e)
    {
        // skip
    }
    
    m_command.expires_from_now(boost::posix_time::seconds(2));
    m_command.async_wait(boost::bind(&CArmAdapter::SendCommandPacket, this, _1));
}

} // namespace device
} // namespace broker
} // namespace freedm

