////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///                 Michael Catanzaro <msc8cc@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of devices and their structures
///
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

#include "CDeviceFactory.hpp"
#include "config.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

// Allocate the static members, setting shared pointers to "null"

#if defined USE_DEVICE_PSCAD
CLineClient::TPointer CDeviceFactory::m_lineClient =
        boost::shared_ptr<CLineClient>();
#elif defined USE_DEVICE_RTDS
CClientRTDS::RTDSPointer CDeviceFactory::m_rtdsClient =
        boost::shared_ptr<CClientRTDS>();
#endif

boost::shared_ptr<CPhysicalDeviceManager> CDeviceFactory::m_manager =
        boost::shared_ptr<CPhysicalDeviceManager>();

RegistryType CDeviceFactory::m_deviceRegistry;

////////////////////////////////////////////////////////////////////////////////
/// SetDeviceManager
///
/// @description Sets the factory's device manager.
/// 
/// @pre None, though presumably the device manager should only be set once.
/// @post The factory will now create devices registered with the specified
///  device manager.
///
/// @param manager the device manager to associate with the factory.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CDeviceFactory::SetDeviceManager(
        boost::shared_ptr<CPhysicalDeviceManager> manager)
{
    m_manager = manager;
}

////////////////////////////////////////////////////////////////////////////////
/// CreateDevice
///
/// @description Translates a string into a class type, then creates a new
///  device of this type with the specified identifier.
/// @ErrorHandling Insufficient: throws a string if the device type is not
///  registered with the factory, or if the factory is not properly configured.
///
/// @pre deviceType and deviceID follow the below specifications.
/// @post Specified device is created and registered with the factory's device
///  manager.
///
/// @param deviceType a string representing the name of the IDevice subclass
///  be created. Should be exactly the same as the portion of the class name
///  after "CDevice".
/// @param deviceID the unique identifier for the device to be created.
///  No other device on this DGI may have this ID.
///
/// @limitations SetPhysicalDeviceManager must be called before any devices
///  are created.  Also, if compiled in RTDS or PSCAD mode, the appropriate
///  client must also be set.
////////////////////////////////////////////////////////////////////////////////
void CDeviceFactory::CreateDevice(const std::string deviceType,
        const Identifier& deviceID)
{
    // Ensure the specified device type exists
    if (m_deviceRegistry.find(deviceType) == m_deviceRegistry.end())
    {
        std::stringstream ss;
        ss << "Attempted to create device of unregistered type "
                << deviceType.c_str();
        throw ss.str();
    }

    // Throws if the factory is not properly configured
    m_deviceRegistry[deviceType](deviceID);
}

////////////////////////////////////////////////////////////////////////////////
/// CreateStructure
///
/// @description Creates the internal structure of a device.  Intended to be
///  immediately passed to a device constructor.
///
/// @pre The factory's device manager has been specified.
/// @post A device structure of the appropriate type is 
///
/// @return an internal device structure as specified by the device manager.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IDeviceStructure::DevicePtr CDeviceFactory::CreateStructure()
{
#if defined USE_DEVICE_PSCAD
    return IDeviceStructure::DevicePtr(
            new CDeviceStructurePSCAD(m_lineClient));
#elif defined USE_DEVICE_RTDS
    return IDeviceStructure::DevicePtr(
            new CDeviceStructureRTDS(m_rtdsClient));
#else
    return IDeviceStructure::DevicePtr(new CDeviceStructureGeneric());
#endif
}

} // namespace device
} // namespace freedm
} // namespace broker
