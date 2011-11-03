///////////////////////////////////////////////////////////////////////////////
/// @file           CGenericFactory.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Factory for production of generic test devices
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missour University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///////////////////////////////////////////////////////////////////////////////

#ifndef C_GENERIC_FACTORY_HPP
#define C_GENERIC_FACTORY_HPP

#include <string>

#include "logger.hpp"
#include "ICreateDevice.hpp"
#include "IPhysicalDevice.hpp"
#include "CPhysicalDeviceManager.hpp"

#include "CGenericDevice.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {
namespace broker {

/// Factory for production of generic test devices
class CGenericFactory : public ICreateDevice
{
public:
    /// Creates an instance of a generic device factory
    CGenericFactory( CPhysicalDeviceManager & p_devman );

    /// Creates the family of generic devices
    virtual void CreateDevice( const std::string & p_type,
        const IPhysicalDevice::Identifier & p_devid );
private:
    /// Convenience type for device pointers
    typedef IPhysicalDevice::DevicePtr DevicePtr;
    
    /// Device manager to store created devices
    CPhysicalDeviceManager & m_manager;
};

} // namespace broker
} // namespace freedm

#endif // C_GENERIC_FACTORY_HPP

