#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace broker {
namespace device {

class IServer
    : private boost::noncopyable
{
public:
    typedef boost::shared_ptr<IServer> Pointer;
    typedef boost::function< void () > ConnectionHandler;

    IServer();

    void RegisterHandler( ConnectionHandler h );

    virtual ~IServer();
protected:
    ConnectionHandler m_handler;
};

} // namespace device
} // namespace broker
} // namespace freedm

