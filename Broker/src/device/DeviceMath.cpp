/// @todo

#include "CLogger.hpp"
#include "device/DeviceMath.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {

/// This file's logger.
CLocalLogger logger(__FILE__);

}

/// @todo
SettingValue SumValues(const SettingValue lhs, const SettingValue rhs)
{
    return lhs + rhs;
}

}
}
}