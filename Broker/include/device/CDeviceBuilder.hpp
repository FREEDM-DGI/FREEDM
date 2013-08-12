#ifndef C_DEVICE_BUILDER_HPP
#define C_DEVICE_BUILDER_HPP

#include <map>
#include <set>
#include <string>
#include <utility>

namespace freedm {
namespace broker {
namespace device {

struct DeviceInfo
{
    std::set<std::string> s_type;
    std::set<std::string> s_state;
    std::set<std::string> s_command;
};

class CDeviceBuilder
{
public:
    CDeviceBuilder(std::string filename);
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

