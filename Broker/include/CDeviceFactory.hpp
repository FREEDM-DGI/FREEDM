////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.hpp
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

#ifndef C_DEVICE_FACTORY_HPP
#define C_DEVICE_FACTORY_HPP

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>

#include "CLineClient.hpp"
#include "CClientRTDS.hpp"
#include "IPhysicalDevice.hpp"
#include "PhysicalDeviceTypes.hpp"
#include "CPhysicalDeviceManager.hpp"

#include "CDeviceStructurePSCAD.hpp"
#include "CDeviceStructureRTDS.hpp"
#include "CDeviceStructureGeneric.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

/// Type of the device registry
typedef std::map<std::string, IDevice(*)(const Identifier&) > RegistryType;

/// Creates devices and their internal structures
class CDeviceFactory
{
    ////////////////////////////////////////////////////////////////////////////
    /// @description A utility class which is used for the registration and 
    ///  construction of Devices.
    /// @limitations If enabled at compilation, the static PSCAD or RTDS client
    ///  must be initialized and connected before any devices are created. The
    ///  physical device manager must similarly be set before any device 
    ///  construction.
    ////////////////////////////////////////////////////////////////////////////

public:
    /// TODO sets the factory's device manager
    static void SetDeviceManager(boost::shared_ptr<CPhysicalDeviceManager>);

#if defined USE_DEVICE_PSCAD
    /// TODO Set the factory's line client
    static void SetLineClient(CLineClient&);
#elif defined USE_DEVICE_RTDS
    /// TODO Set the factory's RTDS client
    static void SetRtdsClient(CClientRTDS&);
#endif

    /// TODO Creates all devices specified by some XML file
    // static void ReadXML(const std::string filename);

    /// TODO Creates a device with the given type and identifier
    static void CreateDevice(const std::string deviceType,
            const Identifier& deviceName);

    /// TODO register device
    //static void RegisterDevice(
    //        std::pair<std::string, IDevice(*)(const Identifier&)>& mapping );

private:
    /// No Device Factory should ever be instantiated
    CDeviceFactory() { };

    ////////////////////////////////////////////////////////////////////////////
    /// CreateDevice<DeviceType>
    ///
    /// @description Creates a DeviceType with the given identifier.
    /// @ErrorHandling Throws a string if the factory is not properly set up.
    ///
    /// @pre No other device on this DGI has the passed deviceID.
    /// @post Specified device is created and registered with the factory's
    ///  device manager.
    ///
    /// @param deviceID the unique identifier for the device to be created.
    ///
    /// @return the device that has been created.
    ///
    /// @limitations SetPhysicalDeviceManager must be called before any devices
    ///  are created.  Also, if compiled in RTDS or PSCAD mode, the appropriate
    ///  client must also be set.
    ////////////////////////////////////////////////////////////////////////////
    template <class DeviceType>
    static void CreateDevice(const Identifier & deviceID)
    {
        if (m_manager == NULL)
        {
            throw "Attempted to create a device, but CDeviceFactory::m_manager"
            += " is uninitialized";
        }
#ifdef USE_DEVICE_PSCAD
        if (!m_lineClient)
        {
            throw "Attempted to create a device, but " +=
                    "CDeviceFactory::m_lineClient is uninitialized";
        }
#elif USE_DEVICE_RTDS
        if (!m_rtdsClient)
        {
            throw "Attempted to create a device, but " +=
                    "CDeviceFactory::m_rtdsClient is uninitialized";
        }
#endif
        IDeviceStructure::DevicePtr ds;
        IDevice::DevicePtr dev;
        // create and register the device structure
        ds = CreateStructure();
        ds->Register(deviceID);
        // create the new device from the structure
        dev = IDevice::DevicePtr(new DeviceType(*m_manager, deviceID, ds));
        // add the device to the manager
        m_manager->AddDevice(dev);
    }

    /// Creates the internal structure of the device
    static IDeviceStructure::DevicePtr CreateStructure();

#if defined USE_DEVICE_PSCAD
    /// Client to the PSCAD simulation server
    static CLineClient::TPointer m_lineClient;
#elif defined USE_DEVICE_RTDS
    /// Client for the RTDS
    static CClientRTDS::RTDSPointer m_rtdsClient;
#endif
    /// Device manager to handle created devices.
    static boost::shared_ptr<CPhysicalDeviceManager> m_manager;
    /// Maps strings of device names to a factory function for that class
    static RegistryType m_deviceRegistry;
};

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_DEVICE_FACTORY_HPP
