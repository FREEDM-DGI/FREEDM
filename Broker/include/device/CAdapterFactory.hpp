////////////////////////////////////////////////////////////////////////////////
/// @file         CAdapterFactory.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Handles the creation of device adapters.
///
/// @functions
///     CAdapterFactory::CreateDevice
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef C_ADAPTER_FACTORY_HPP
#define C_ADAPTER_FACTORY_HPP

#include "CDeviceManager.hpp"
#include "CLogger.hpp"
#include "IAdapter.hpp"

#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger AdapterFactoryLogger(__FILE__);
}

/// Converts a string identifier into a templated function call.
#define REGISTER_DEVICE_CLASS(SUFFIX) CAdapterFactory::Instance().\
RegisterDeviceClass(#SUFFIX, &CAdapterFactory::CreateDevice<CDevice##SUFFIX>)

/// Handles the creation of adapters and their associated devices.
////////////////////////////////////////////////////////////////////////////////
/// Singleton factory that creates, stores, and runs new device adapters.
///
/// @limitations The factory must be initialized by CAdapterFactory::Initialize
/// before new adapters can be created.  Otherwise, the factory will throw a
/// std::runtime_error exception.  This class is not thread safe.
////////////////////////////////////////////////////////////////////////////////
class CAdapterFactory
    : private boost::noncopyable
{
public:
   /// Gets the static instance of the factory.
    static CAdapterFactory & Instance();
    
    /// Gives the factory access to the device manager.
    void Initialize(CDeviceManager::Pointer manager);
    
    /// Creates a new adapter and its associated devices.
    void CreateAdapter(const boost::property_tree::ptree & p);

    /// @todo should this be public?
    void RegisterPhysicalDevices();
    
    /// Destructs the factory.
    ~CAdapterFactory();
private:
    /// Type of the functions used to create new devices.
    typedef void
    (CAdapterFactory::*FactoryFunction)(std::string, IAdapter::Pointer adapter);
     
    /// Constructs the factory.
    CAdapterFactory();
    
    /// Registers compiled device classes with the factory.
    void RegisterDevices();
    
    /// Registers a device class with the factory.
    void RegisterDeviceClass(std::string key, FactoryFunction function);
    
    /// Constructs an adapter without any devices.
    IAdapter::Pointer CreateAdapter(std::string name, std::string type,
                                    const boost::property_tree::ptree & p);
    
    /// Creates a device and registers it with the system.
    void CreateDevice(std::string name, std::string type, 
                      IAdapter::Pointer adapter);
    
    /// Creates a device and registers it with the system.
    template <class DeviceType>
    void CreateDevice(std::string name, IAdapter::Pointer adapter);
    
    /// Device manager used to register new devices.
    CDeviceManager::Pointer m_manager;
    
    /// Set of adapters created by the factory.
    std::map<std::string, IAdapter::Pointer> m_adapter;

    /// Set of device classes registered by the factory.
    std::map<std::string, FactoryFunction> m_registry;
    
    /// I/O service shared by the adapters.
    boost::asio::io_service m_ios;
    
    /// Thread to run the i/o service
    boost::thread m_thread;
};

////////////////////////////////////////////////////////////////////////////////
/// Creates a device of DeviceType with the given name and registers it with
/// the factory's device manager.  The device is constructed to access the
/// passed adapter.
///
/// @ErrorHandling Throws a std::runtime_error if either the factory has not
/// been initialized with CAdapterFactory::Initialize or a device exists with
/// the provided name.
/// @pre The provided adapter must not be null.
/// @post The new device is registered with the device manager.
/// @post The provided adapter is registered with the new device.
/// @param name The unique identifier for the device to be created.
/// @param adapter The adapter the new device should access its data through.
///
/// @limitations CAdapterFactory::Initialize must be called before this func.
////////////////////////////////////////////////////////////////////////////////
template <class DeviceType>
void CAdapterFactory::CreateDevice(std::string name, IAdapter::Pointer adapter)
{
    AdapterFactoryLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( !m_manager )
    {
        throw std::runtime_error("Adapter factory has not been initialized.");
    }
    
    if( m_manager->DeviceExists(name) )
    {
        throw std::runtime_error("The device " + name + " already exists.");
    }
    
    IDevice::Pointer device(new DeviceType(name, adapter));
    m_manager->AddDevice(device);
    
    AdapterFactoryLogger.Info << "Created new device: " << name << std::endl;
}

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_ADAPTER_FACTORY_HPP
