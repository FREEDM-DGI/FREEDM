///////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactoryGeneric.cpp
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

#include "CDeviceFactoryGeneric.hpp"

namespace freedm {
namespace broker {

/// Creates an instance of a generic device factory
CDeviceFactoryGeneric::CDeviceFactoryGeneric( CPhysicalDeviceManager & p_devman )
    : m_manager(p_devman)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
}

/// Creates the family of generic devices
void CDeviceFactoryGeneric::CreateDevice( const std::string & p_type,
    const IPhysicalDevice::Identifier & p_devid )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    IPhysicalDevice::DevicePtr device;

    // do work
    if( p_type == "solar" )
    {
        device = IPhysicalDevice::DevicePtr(new devices::IDevicePVGeneric(m_manager, p_devid));
    }
    else if( p_type == "load" )
    {
        device = IPhysicalDevice::DevicePtr(new devices::IDeviceLoadGeneric(m_manager, p_devid));
    }
    else if( p_type == "battery" )
    {
        device = IPhysicalDevice::DevicePtr(new devices::IDeviceBatteryGeneric(m_manager, p_devid));
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
