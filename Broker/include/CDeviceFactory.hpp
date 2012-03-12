////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of devices and their structures.
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
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_FACTORY_HPP
#define C_DEVICE_FACTORY_HPP

#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>

#include "CClientRTDS.hpp"
#include "CDeviceStructureGeneric.hpp"
#include "CDeviceStructurePSCAD.hpp"
#include "CDeviceStructureRTDS.hpp"
#include "CLineClient.hpp"
#include "CPhysicalDeviceManager.hpp"
#include "IPhysicalDevice.hpp"

namespace freedm
{
namespace broker
{
namespace device
{

//int regDummy;
// Must initialize a variable to use this outside of functions.)
#define REGISTER_DEVICE_CLASS(SUFFIX) int freedm::broker::device::regDummy = \
freedm::broker::device::CDeviceFactory::instance().RegisterDeviceClass( \
#SUFFIX, &freedm::broker::device::CDeviceFactory::CreateDevice< \
freedm::broker::device::CDevice##SUFFIX>);

class CDeviceFactory;

/// Type of the factory functions.
typedef void (CDeviceFactory::*FactoryFunction )(const Identifier&);

/// Type of the device registry.
typedef std::map<const std::string, FactoryFunction> DeviceRegistryType;

/// Creates devices and their internal structures.
class CDeviceFactory : private boost::noncopyable
{
    ////////////////////////////////////////////////////////////////////////////
    /// CDeviceFactory
    ///
    /// @description Singleton factory that accepts registrations of device
    ///  classes and creates instances of registered classes as requested.
    ///  Instances are themselves registered in the factory's device manager.
    ///
    /// @limitations The singleton instance must be configured with the init
    ///  function before any devices are created. It is, however, safe to
    ///  register devices before init is called.
    ////////////////////////////////////////////////////////////////////////////

public:
    /// Retrieves the static instance of the device factory class.
    static CDeviceFactory& instance();

    /// Loads the factory with device manager and networking data.
    void init(CPhysicalDeviceManager& manager,
            boost::asio::io_service& ios, const std::string host,
            const std::string port, const std::string xml);

    /// Registers a device class with the factory.
    int RegisterDeviceClass(const std::string key, FactoryFunction value);

    /// Creates a device with the given type and identifier.
    void CreateDevice(const Identifier& deviceID, const std::string deviceType);

    /// Creates and registers DeviceID with the given identifier.
    template <class DeviceType>
    void CreateDevice(const Identifier& deviceID);

    // TODO Creates all devices specified by some XML file.
    // void ReadXML(const std::string filename);

private:
    /// Constructs the device factory.
    CDeviceFactory();

    /// Creates the internal structure of the device.
    IDeviceStructure::DevicePtr CreateStructure() const;

    /// Client to the PSCAD simulation server.
    CLineClient::TPointer m_lineClient;

    /// Client for the RTDS.
    CClientRTDS::RTDSPointer m_rtdsClient;

    /// Device manager to handle created devices.
    CPhysicalDeviceManager* m_manager;

    /// Maps strings of device names to a factory function for that class.
    DeviceRegistryType m_registry;

    /// Used to indicate whether or not init has been called on this factory.
    bool m_initialized;
};

////////////////////////////////////////////////////////////////////////////////
/// CreateDevice<DeviceType>
///
/// @description Creates a DeviceType with the given identifier and
///  registers it with the factory's device manager. It is intended that this
///  function not be called directly, but rather registered via function pointer
///  through CDeviceFactory::RegisterDeviceClass and called indirectly through
///  the CreateDevice function taking the string parameters. However, this
///  function is intentionally public and is safe to use directly.
///
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
/// @limitations CDeviceFactory::init must be called before any devices are
///  created.
////////////////////////////////////////////////////////////////////////////////
template <class DeviceType>
void CDeviceFactory::CreateDevice(const Identifier& deviceID)
{
    if (!m_initialized)
    {
        throw "CDeviceFactory::CreateDevice (private) called before init";
    }
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

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_DEVICE_FACTORY_HPP
