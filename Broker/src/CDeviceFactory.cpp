///////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Stores and manages an instance of a device factory
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

#include "CDeviceFactory.hpp"
#include "config.hpp"

namespace freedm {
namespace broker {

/// Creates an instance of a device factory
CDeviceFactory::CDeviceFactory( CPhysicalDeviceManager & p_devman,
    boost::asio::io_service & p_ios, const std::string & p_host,
    const std::string & p_port )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
#if defined USE_DEVICE_PSCAD
    Logger::Info << "Initialized to use PSCAD devices" << std::endl;
    m_factory.reset(new CPSCADFactory( p_devman, p_ios, p_host, p_port ));
#else
    Logger::Info << "Initialized to use generic devices" << std::endl;
    m_factory.reset(new CGenericFactory( p_devman ));
#endif
}

/// Delegates the creation of a device to the managed device factory
void CDeviceFactory::CreateDevice( const std::string & p_type,
        const IPhysicalDevice::Identifier & p_devid )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    // delegate creation to the factory
    m_factory->CreateDevice( p_type, p_devid );
}

} // namespace broker
} // namespace freedm
