#ifndef C_DEVICE_HPP
#define C_DEVICE_HPP

#include "IAdapter.hpp"

#include <set>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
namespace broker {
namespace device {

struct DeviceInfo
{
    std::set<std::string> s_type;
    std::set<std::string> s_state;
    std::set<std::string> s_command;
};

class CDevice
    : private boost::noncopyable
{
public:
    typedef boost::shared_ptr<CDevice> Pointer;

    CDevice(std::string id, DeviceInfo info, IAdapter::Pointer adapter);

    std::string GetID() const;

    bool HasType(std::string type) const;
    bool HasState(std::string signal) const;
    bool HasCommand(std::string signal) const;

    SignalValue GetState(std::string signal) const;
    SignalValue GetCommand(std::string signal, bool override = false) const;

    void SetCommand(std::string signal, SignalValue value);

    void ClearCommands();
private:
    std::string m_devid;
    DeviceInfo m_devinfo;
    IAdapter::Pointer m_adapter;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_HPP

