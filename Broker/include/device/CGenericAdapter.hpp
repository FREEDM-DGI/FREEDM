/* 
 * File:   CGenericAdapter.hpp
 * Author: michael
 *
 * Created on May 2, 2012, 7:22 PM
 */

#include "IPhysicalAdapter.hpp"

#ifndef CGENERICADAPTER_HPP
#define	CGENERICADAPTER_HPP

namespace freedm {
namespace broker {
namespace device {

/// @TODO the whole thing
class CGenericAdapter : public IPhysicalAdapter
{
    /// Retrieve a value from a device.
    virtual SettingValue Get(const Identifier device,
            const SettingKey key) const
    {
        return 0.0;
    }

    /// Set a value on a device.
    virtual void Set(const Identifier device, const SettingKey key,
            const SettingValue value) { }
};

}
}
}

#endif	/* CGENERICADAPTER_HPP */

