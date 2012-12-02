#ifndef C_ARM_ADAPTER_HPP
#define C_ARM_ADAPTER_HPP

#include "IBufferAdapter.hpp"
#include "ITcpAdapter.hpp"
#include "CTcpServer.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

class CArmAdapter
    : public IBufferAdapter
    , public ITcpAdapter
{
public:
    typedef boost::shared_ptr<CArmAdapter> Pointer;

    static IAdapter::Pointer Create(boost::asio::io_service & service,
        boost::property_tree::ptree & p);

    void Start();
    void Quit();
    void Timeout(const boost::system::error_code & e);

    ~CArmAdapter();
private:
    CArmAdapter(boost::asio::io_service & service,
        boost::property_tree::ptree & p);

    void HandleConnection(IServer::Pointer connection);

    boost::asio::deadline_timer m_timer;
    CTcpServer::Pointer m_server;
    unsigned short m_port;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_ARM_ADAPTER_HPP

