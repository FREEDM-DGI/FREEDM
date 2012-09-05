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

#include "CLogger.hpp"
#include "IAdapter.hpp"
#include "CDeviceManager.hpp"

#include <map>
#include <string>
#include <stdexcept>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger AdapterFactoryLogger(__FILE__);
}

/// Converts a string identifier into a templated function call.
#define REGISTER_DEVICE_CLASS(SUFFIX) \
RegisterDeviceClass(#SUFFIX, &CAdapterFactory::CreateDevice<CDevice##SUFFIX>)

/// Handles the creation of adapters and their associated devices.
////////////////////////////////////////////////////////////////////////////////
/// Singleton factory that creates, stores, and runs new device adapters.
///
/// @limitations This class is not thread safe.
////////////////////////////////////////////////////////////////////////////////
class CAdapterFactory
    : private boost::noncopyable
{
public:
    /// Gets the static instance of the factory.
    static CAdapterFactory & Instance();
    
    /// Creates a new adapter and its associated devices.
    void CreateAdapter(const boost::property_tree::ptree & p);
    
    /// Destructs the factory.
    ~CAdapterFactory();
private:
    /// Type of the functions used to create new devices.
    typedef void (CAdapterFactory::*FactoryFunction)
            (std::string, IAdapter::Pointer adapter);
     
    /// Constructs the factory.
    CAdapterFactory();
    
    /// Registers compiled device classes with the factory.
    void RegisterDevices();
    
    /// Registers a single device class with the factory.
    void RegisterDeviceClass(std::string key, FactoryFunction function);
    
    /// Initializes the devices stored on an adapter.
    void InitializeAdapter(IAdapter::Pointer adapter,
            const boost::property_tree::ptree & p);
    
    /// Creates a device and registers it with the system.
    void CreateDevice(std::string name, std::string type, 
            IAdapter::Pointer adapter);
    
    /// Creates a device and registers it with the system.
    template <class DeviceType>
    void CreateDevice(std::string name, IAdapter::Pointer adapter);
    
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
/// @ErrorHandling Throws a std::runtime_error if a device has the given name.
/// @pre The provided adapter must not be null.
/// @post The new device is registered with the device manager.
/// @post The provided adapter is registered with the new device.
/// @param name The unique identifier for the device to be created.
/// @param adapter The adapter the new device should access its data through.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <class DeviceType>
void CAdapterFactory::CreateDevice(std::string name, IAdapter::Pointer adapter)
{
    AdapterFactoryLogger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( CDeviceManager::Instance().DeviceExists(name) )
    {
        throw std::runtime_error("The device " + name + " already exists.");
    }
    
    IDevice::Pointer device(new DeviceType(name, adapter));
    CDeviceManager::Instance().AddDevice(device);
    
    AdapterFactoryLogger.Info << "Created new device: " << name << std::endl;
}

} // namespace device
} // namespace freedm
} // namespace broker

#endif // C_ADAPTER_FACTORY_HPP
