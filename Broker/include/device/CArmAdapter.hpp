#ifndef C_ARM_ADAPTER_HPP
#define C_ARM_ADAPTER_HPP

#include "IBufferAdapter.hpp"
#include "ITcpAdapter.hpp"
#include "CTcpServer.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace broker {
namespace device {

class CArmAdapter
    : public IBufferAdapter
    , public ITcpAdapter
{
public:
    /// Convenience type for a shared pointer to self.
    typedef boost::shared_ptr<CArmAdapter> Pointer;

    static IAdapter::Pointer Create(boost::asio::io_service & service,
            boost::property_tree::ptree & p);

    unsigned short GetStatePort() const;
    unsigned short GetHeartbeatPort() const;
    void Heartbeat();
    void Start();
    void Quit();
    void Send(const std::string data);
    void Timeout(const boost::system::error_code & e);

    ~CArmAdapter();
private:
    CArmAdapter(boost::asio::io_service & service,
            boost::property_tree::ptree & p);

    void HandleHeartbeat(IServer::Pointer connection);
    void HandleState(IServer::Pointer connection);

    boost::asio::deadline_timer m_timer;
    CTcpServer::Pointer m_HeartbeatServer;
    CTcpServer::Pointer m_StateServer;
    unsigned short m_HeartbeatPort;
    unsigned short m_StatePort;
    std::string m_identifier;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_ARM_ADAPTER_HPP

