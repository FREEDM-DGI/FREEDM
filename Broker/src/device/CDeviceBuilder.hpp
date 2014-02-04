#ifndef C_DEVICE_BUILDER_HPP
#define C_DEVICE_BUILDER_HPP

#include "CDevice.hpp"

#include <map>
#include <set>
#include <string>
#include <utility>

namespace freedm {
namespace broker {
namespace device {

class CDeviceBuilder
{
public:
    CDeviceBuilder() {}
    CDeviceBuilder(std::string filename);
    DeviceInfo GetDeviceInfo(std::string type);
    CDevice::Pointer CreateDevice(std::string id, std::string type, IAdapter::Pointer adapter);
private:
    struct BuildVars
    {
        typedef std::pair<std::string, std::string> StrPair;
        std::map<StrPair, std::string> s_signal_conflict;
        std::set<std::string> s_uninitialized_type;
    };

    void ExpandInfo(std::string target, std::set<std::string> path, BuildVars & vars);

    std::map<std::string, DeviceInfo> m_type_to_info;
};

} // namespace device
} // namespace broker
} // namespace freedm

#endif // C_DEVICE_BUILDER_HPP

