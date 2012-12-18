#ifndef C_ARM_ADAPTER_HPP
#define C_ARM_ADAPTER_HPP

#include "IBufferAdapter.hpp"
#include "CTcpServer.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace broker {
namespace device {

class CArmAdapter
    : public IBufferAdapter
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<CArmAdapter> Pointer;

    static IAdapter::Pointer Create(boost::asio::io_service & service,
            boost::property_tree::ptree & p);
    
    void Start();
    void Heartbeat();
    unsigned short GetPortNumber() const;
    void Timeout(const boost::system::error_code & e);

    ~CArmAdapter();
private:
    CArmAdapter(boost::asio::io_service & service,
            boost::property_tree::ptree & p);
            
    void HandleMessage(IServer::Pointer connection);
    
    void SendCommandPacket();
    
    boost::asio::deadline_timer m_heartbeat;
    
    boost::asio::deadline_timer m_command;
        
    std::string m_identifier;
    
    unsigned short m_port;
    
    bool m_initialized;
    
    CTcpServer::Pointer m_server;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_ARM_ADAPTER_HPP
