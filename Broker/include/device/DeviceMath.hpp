/* 
 * File:   DeviceMath.hpp
 * Author: michael
 *
 * Created on June 2, 2012, 12:46 PM
 * 
 * @todo
 */

#ifndef DEVICEMATH_HPP
#define	DEVICEMATH_HPP

// @todo move device typedefs from IPhysicalAdapter somewhere better
#include "IPhysicalAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

SettingValue SumValues(const SettingValue lhs, const SettingValue rhs);

}
}
}

#endif	/* DEVICEMATH_HPP */

