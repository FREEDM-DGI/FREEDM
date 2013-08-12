#include "CDevice.hpp"
#include "CLogger.hpp"

#include <stdexcept>

#include <boost/foreach.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
CLocalLogger Logger(__FILE__);
}

CDevice::CDevice(std::string id, DeviceInfo info, IAdapter::Pointer adapter)
    : m_devid(id)
    , m_devinfo(info)
    , m_adapter(adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

std::string CDevice::GetID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devid;
}

bool CDevice::HasType(std::string type) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_type.count(type) > 0;
}

bool CDevice::HasState(std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_state.count(signal) > 0;
}

bool CDevice::HasCommand(std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_command.count(signal) > 0;
}

SignalValue CDevice::GetState(std::string signal) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !HasState(signal) )
    {
        throw std::runtime_error("bad state signal");
    }

    //return m_adapter->GetState(m_devid, signal);
}

SignalValue CDevice::GetCommand(std::string signal, bool override) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !HasCommand(signal) )
    {
        throw std::runtime_error("bad command signal");
    }

/*
    SignalValue result = m_adapter->GetCommand(m_devid, signal);
    
    if( override && HasState(signal) && result == NULL_COMMAND )
    {
        result = m_adapter->GetState(m_devid, signal);
    }
    return result;
*/
}

void CDevice::SetCommand(std::string signal, SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !HasCommand(signal) )
    {
        throw std::runtime_error("bad command signal");
    }

    //m_adapter->SetCommand(m_devid, signal, value);
}

void CDevice::ClearCommands()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    BOOST_FOREACH(std::string signal, m_devinfo.s_command)
    {
        //m_adapter->SetCommand(m_devid, signal, NULL_COMMAND);
    }
}

} // namespace device
} // namespace broker
} // namespace freedm

