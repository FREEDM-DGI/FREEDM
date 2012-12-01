#ifndef C_TCP_SERVER_HPP
#define C_TCP_SERVER_HPP

#include "IServer.hpp"
#include <boost/asio.hpp>

namespace freedm {
namespace broker {
namespace device {

class CTcpServer
    : public IServer
{
public:
    typedef boost::shared_ptr<CTcpServer> Pointer;

    static Pointer Create( boost::asio::io_service & ios, unsigned short port );

    std::string ReceiveData();
    void SendData(const std::string str);
    std::string GetHostname() const;

    virtual ~CTcpServer();
private:
    CTcpServer( boost::asio::io_service & ios, unsigned short port );

    void StartAccept();
    void HandleAccept( const boost::system::error_code & error );

    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    unsigned short m_id;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_TCP_SERVER_HPP

