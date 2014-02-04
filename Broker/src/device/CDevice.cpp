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

    return m_adapter->GetState(m_devid, signal);
}

std::set<std::string> CDevice::GetStateSet() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_command;
}

std::set<std::string> CDevice::GetCommandSet() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_devinfo.s_state;
}

void CDevice::SetCommand(std::string signal, SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !HasCommand(signal) )
    {
        throw std::runtime_error("bad command signal");
    }

    m_adapter->SetCommand(m_devid, signal, value);
}

} // namespace device
} // namespace broker
} // namespace freedm

