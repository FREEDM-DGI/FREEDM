////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of devices and their structures
///
/// @license
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_FACTORY_HPP
#define C_DEVICE_FACTORY_HPP

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>

#include "CLineClient.hpp"
#include "IPhysicalDevice.hpp"
#include "PhysicalDeviceTypes.hpp"
#include "CPhysicalDeviceManager.hpp"

#include "CDeviceStructurePSCAD.hpp"
#include "CDeviceStructureGeneric.hpp"

namespace freedm {
namespace broker {
namespace device {

/// Creates devices and their internal structures
class CDeviceFactory
    : private boost::noncopyable
{
public:
    /// Creates an instance of a device factory
    CDeviceFactory( CPhysicalDeviceManager & manager,
        boost::asio::io_service & ios, const std::string & host,
        const std::string & port );
    
    /// Creates a device with the given identifier
    template <class DeviceType>
    void CreateDevice( const Identifier & device )
    {
        IDeviceStructure::DevicePtr ds;
        CDevice::DevicePtr dev;
        
        // create and register the device structure
        ds = CreateStructure();
        ds->Register(device);
        
        // create the new device from the structure
        dev = CDevice::DevicePtr( new DeviceType( m_manager, device, ds ) );
        
        // add the device to the manager
        m_manager.AddDevice(dev);
    }
private:
    /// Creates the internal structure of the device
    IDeviceStructure::DevicePtr CreateStructure();
    
    /// Device manager to handle created devices
    CPhysicalDeviceManager & m_manager;
    
    /// Client to the PSCAD simulation server
    CLineClient::TPointer m_client;
};

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_DEVICE_FACTORY_HPP
