#ifndef I_SERVER_HPP
#define I_SERVER_HPP

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace freedm {
namespace broker {
namespace device {

class IServer
    : private boost::noncopyable
    , public boost::enable_shared_from_this<IServer>
{
public:
    typedef boost::shared_ptr<IServer> Pointer;
    typedef boost::function<void (Pointer)> ConnectionHandler;

    IServer();

    void RegisterHandler( ConnectionHandler h );

    virtual std::string ReceiveData() = 0;
    virtual void SendData(const std::string str) = 0;

    virtual ~IServer();
protected:
    ConnectionHandler m_handler;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // I_SERVER_HPP

