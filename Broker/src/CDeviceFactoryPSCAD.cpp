///////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactoryPSCAD.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Factory for production of PSCAD-enabled devices
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

#include "CDeviceFactoryPSCAD.hpp"

namespace freedm {
namespace broker {

/// Creates an instance of a PSCAD device factory
CDeviceFactoryPSCAD::CDeviceFactoryPSCAD( CPhysicalDeviceManager & p_devman,
    boost::asio::io_service & p_ios, const std::string & p_host,
    const std::string & p_port )
    : m_manager(p_devman)
    , m_client(CLineClient::Create(p_ios))
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    // connect to the simulation server
    m_client->Connect(p_host,p_port);
}

/// Creates the family of PSCAD-enabled devices
void CDeviceFactoryPSCAD::CreateDevice( const std::string & p_type,
    const IPhysicalDevice::Identifier & p_devid )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    
    IPhysicalDevice::DevicePtr device;
    
    // create the device dependent on its type
    //  we really need a way to make this whole slop prettier
    if( p_type == "solar" )
    {
        device = IPhysicalDevice::DevicePtr(new devices::IDevicePVPSCAD(m_client, m_manager, p_devid));
    }
    else if( p_type == "load" )
    {
        device = IPhysicalDevice::DevicePtr(new devices::IDeviceLoadPSCAD(m_client, m_manager, p_devid));
    }
    else if( p_type == "battery" )
    {
        device = IPhysicalDevice::DevicePtr(new devices::IDeviceBatteryPSCAD(m_client, m_manager, p_devid));
    }
    else
    {
        Logger::Error << "Cannot add " << p_type << " device" << std::endl;
        return;
    }
    m_manager.AddDevice(device);
    
    Logger::Debug << "Added " << p_type << " device " << p_devid << std::endl;
}

} // namespace broker
} // namespace freedm
