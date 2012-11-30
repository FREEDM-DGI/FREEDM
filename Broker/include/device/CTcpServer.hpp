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

    virtual ~CTcpServer();
private:
    CTcpServer( boost::asio::io_service & ios, unsigned short port );

    void StartAccept();
    void HandleAccept( const boost::system::error_code & error );

    unsigned short m_id;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::acceptor m_acceptor;
};

} // namespace device
} // namespace broker
} // namespace freedm

