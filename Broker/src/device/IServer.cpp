#include "IServer.hpp"
#include "CLogger.hpp"
#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

namespace // unnamed
{
    CLocalLogger Logger(__FILE__);
}

IServer::IServer()
    : m_handler(0)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

IServer::~IServer()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

void IServer::RegisterHandler( ConnectionHandler h )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !m_handler.empty() )
    {
        throw std::runtime_error(
            "Attempted to override an IServer connection handler.");
    }

    if( h.empty() )
    {
        throw std::runtime_error(
            "IServer received an empty connection handler.");
    }

    m_handler = h;
}

} // namespace device
} // namespace broker
} // namespace freedm

