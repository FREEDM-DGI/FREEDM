///////////////////////////////////////////////////////////////////////////////
/// @file           CGenericFactory.cpp
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

#include "CGenericFactory.hpp"

namespace freedm {
namespace broker {

/// Creates an instance of a generic device factory
CGenericFactory::CGenericFactory( CPhysicalDeviceManager & p_devman )
    : m_manager(p_devman)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
}

/// Creates the family of generic devices
void CGenericFactory::CreateDevice( const std::string & p_type,
    const IPhysicalDevice::Identifier & p_devid )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    DevicePtr device;
    
    // uncomment this when IPhysicalDevice is fixed . . .
    if( p_type == "generic" )
    {
        //device = DevicePtr(new CGenericDevice( m_manager, p_devid ));
    }
    else
    {
        Logger::Error << "Cannot add " << p_type << " device" << std::endl;
        return;
    }
    //m_manager.AddDevice(device);
    
    Logger::Debug << "Added " << p_type << " device " << p_devid << std::endl;
}

} // namespace broker
} // namespace freedm
