////////////////////////////////////////////////////////////////////////////////
/// @file           CDeviceFactory.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of devices and their structures.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files.
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef C_DEVICE_FACTORY_HPP
#define C_DEVICE_FACTORY_HPP

#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>

#include "CGenericAdapter.hpp"
#include "CLogger.hpp"
#include "CPhysicalDeviceManager.hpp"
#include "CPscadAdapter.hpp"
#include "CRtdsAdapter.hpp"

namespace freedm {
namespace broker {
namespace device {

/// This file's logger.
static CLocalLogger CDeviceFactoryHPPLogger(__FILE__);

#define REGISTER_DEVICE_CLASS(SUFFIX) CDeviceFactory::instance().\
RegisterDeviceClass(#SUFFIX, &CDeviceFactory::CreateDevice<CDevice##SUFFIX>)

class CDeviceFactory;

/// Type of the factory functions.
typedef void (CDeviceFactory::*FactoryFunction )(const Identifier);

/// Type of the device registry.
typedef std::map<const std::string, FactoryFunction> DeviceRegistryType;

/// Handles the creation of devices and their structures.
////////////////////////////////////////////////////////////////////////////////
/// Singleton factory that accepts registrations of device classes and creates
/// instances of registered classes as requested. (Instances are themselves
/// registered in the factory's device manager.) To register a device class,
/// rather than mess with member function pointer syntax, simply call the
/// REGISTER_DEVICE_CLASS macro with the name of the device class to be created,
/// less the "CDevice" prefix. The macro should then be followed by a semicolon.
/// For example, to register the class CDeviceSST, call 
/// <code>REGISTER_DEVICE_CLASS(SST);</code>
///
/// @limitations The singleton instance must be configured with the init
///  function before any devices are created. It is, however, safe to register
///  devices before init is called. The REGISTER_DEVICE_CLASS macro must be
///  called from within a function, before any devices of the specified class
///  are created and after the class has been fully declared.
////////////////////////////////////////////////////////////////////////////////
class CDeviceFactory : private boost::noncopyable
{
public:
    /// Retrieves the static instance of the device factory class.
    static CDeviceFactory& instance();

    /// Loads the factory with device manager and networking data.
    void init(CPhysicalDeviceManager::ManagerPtr manager,
            boost::asio::io_service& ios, const std::string fpgaCfgFile,
            const std::string host, const std::string port);

    /// Registers a device class with the factory.
    void RegisterDeviceClass(const std::string key, FactoryFunction value);

    /// Creates a device and registers it with the factory's device manager.
    void CreateDevice(const Identifier deviceID, const std::string deviceType);

    /// Creates a device and registers it with the factory's device manager.
    template <class DeviceType>
    void CreateDevice(const Identifier deviceID);

    /// Creates all devices specified by a vector.
    void CreateDevices(const std::vector<std::string>& deviceList);

private:
    /// Constructs the device factory.
    CDeviceFactory();

    /// Device adapter to attach to created devices.
    IPhysicalAdapter::Pointer m_adapter;

    /// Device manager to handle created devices.
    CPhysicalDeviceManager::ManagerPtr m_manager;

    /// Maps strings of device names to a factory function for that class.
    DeviceRegistryType m_registry;

    /// Used to indicate whether or not init has been called on this factory.
    bool m_initialized;
    
#ifdef USE_DEVICE_RTDS
    /// IO service for RTDS adapters, @todo manage own memory!!
    boost::asio::io_service *m_ios;
    
    /// Name of the file containing the FPGA message specification 
    std::string m_fpgaCfgFile;
#endif
};

////////////////////////////////////////////////////////////////////////////////
/// Creates a DeviceType with the given identifier and registers it with the
/// factory's device manager. It is intended that this function not be called
/// directly, but rather registered via function pointer through
/// CDeviceFactory::RegisterDeviceClass and called indirectly through the
/// CreateDevice function taking the string parameters. However, this function
/// is intentionally public (otherwise, it would be impossible to utilize
/// the REGISTER_DEVICE_CLASS macro outside of the factory) and is safe to use
/// directly.
///
/// @ErrorHandling Throws an exception if the factory is not initialized.
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
void CDeviceFactory::CreateDevice(const Identifier deviceID)
{
    CDeviceFactoryHPPLogger.Debug << __PRETTY_FUNCTION__ << std::endl;
    if (!m_initialized)
    {
        std::stringstream ss;
        ss << __PRETTY_FUNCTION__ << " called before factory init" << std::endl;
        throw std::runtime_error(ss.str());
    }
    IDevice::DevicePtr dev(new DeviceType(deviceID, m_adapter));
    m_manager->AddDevice(dev);
}

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_DEVICE_FACTORY_HPP
