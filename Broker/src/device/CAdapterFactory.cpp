///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapterFactory.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Handles the creation of device adapters.
///
/// @functions
///     CAdapterFactory::CAdapterFactory
///     CAdapterFactory::~CAdapterFactory
///     CAdapterFactory::Instance
///     CAdapterFactory::CreateAdapter
///     CAdapterFactory::RegisterDeviceClass
///     CAdapterFactory::InitializeBuffer
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
///////////////////////////////////////////////////////////////////////////////

#include "CAdapterFactory.hpp"
#include "IBufferAdapter.hpp"
#include "CPscadAdapter.hpp"
#include "CRtdsAdapter.hpp"

#include <set>
#include <utility>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs an uninitialized factory.
///
/// @pre None.
/// @post Registers the recognized device classes.
/// @post Launches an i/o service on a separate thread.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CAdapterFactory::CAdapterFactory()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    RegisterDevices();
    m_thread = boost::thread(boost::bind(
            &boost::asio::io_service::run, &m_ios));
    Logger.Status << "Started the adapter i/o service." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Stops the i/o service and waits for its thread to complete.
///
/// @pre None.
/// @post m_ios is stopped and the object is destroyed.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CAdapterFactory::~CAdapterFactory()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_ios.stop();
    Logger.Status << "Stopped the adapter i/o service." << std::endl;
    
    Logger.Notice << "Blocking until thread finishes execution." << std::endl;
    m_thread.join();
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieves the singleton factory instance.
///
/// @pre None.
/// @post Creates a new factory on the first call.
/// @return The global instance of CAdapterFactory.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CAdapterFactory & CAdapterFactory::Instance()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    static CAdapterFactory instance;
    return instance;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new adapter and all of its devices.  The adapter is registered
/// with each device, and each device is registered with the stored device
/// manager.  The adapter is configured to recognize its own device signals,
/// and started when the configuration is complete.
///
/// @ErrorHandling Throws a std::runtime_error if the property tree is bad.
/// @pre The adapter must not begin work until IAdapter::Start.
/// @pre The adapter's devices must not be specified in other adapters.
/// @post Calls CAdapterFactory::CreateDevice to create each device.
/// @post Starts the adapter through IAdapter::Start.
/// @param p The property tree that specifies a single adapter.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::CreateAdapter(const boost::property_tree::ptree & p)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::property_tree::ptree subtree;
    IAdapter::Pointer adapter;
    std::string name, type;
    
    // extract the properties
    try
    {
        name    = p.get<std::string>("<xmlattr>.name");
        type    = p.get<std::string>("<xmlattr>.type");
        subtree = p.get_child("info");
    }
    catch( std::exception & e )
    {
        throw std::runtime_error("Failed to create adapter: "
                + std::string(e.what()));
    }
    
    Logger.Debug << "Building " << type << " adapter " << name << std::endl;
    
    // range check the properties
    if( name.empty() )
    {
        throw std::runtime_error("Tried to create an adapter without a name.");
    }
    else if( m_adapter.count(name) > 0 )
    {
        throw std::runtime_error("Multiple adapters share name " + name);
    }
    
    // create the adapter
    if( type == "pscad" )
    {
        adapter = CPscadAdapter::Create(m_ios, subtree);
    }
    else if( type == "rtds" )
    {
        adapter = CRtdsAdapter::Create(m_ios, subtree);
    }
    else
    {
         throw std::runtime_error("Attempted to create adapter of an "
            + std::string("unrecognized type: ") + type);
    }
    
    // store the adapter
    InitializeAdapter(adapter, p);
    m_adapter[name] = adapter;
    Logger.Info << "Created the " << type << " adapter " << name << std::endl;
    
    // signal construction complete
    adapter->Start();
}

///////////////////////////////////////////////////////////////////////////////
/// Registers a create device function with the factory for the given key.
///
/// @ErrorHandling Throws a std::runtime_error if the key already exists.
/// @pre The key should not already be registered with the factory.
/// @post Stores the function in m_registry for internal use.
/// @param key The unique identifier associated with the function.
/// @param function The function to register with the factory.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::RegisterDeviceClass(std::string key,
        FactoryFunction function)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( m_registry.count(key) > 0 )
    {
        throw std::runtime_error("Device type " + key + " already registered.");
    }

    m_registry.insert(std::make_pair(key, function));
    Logger.Info << "Registered the device class " << key << "." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Initializes an adapter to contain a set of device signals.
///
/// @ErrorHandling Throws a std::runtime_error if the adapter is empty or the
/// property tree has a bad specification format.
/// @pre The property tree must contain an adapter specification.
/// @post Associates a set of device signals with the passed adapter.
/// @param adapter The adapter to initialize.
/// @param p The property tree that contains the buffer data.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::InitializeAdapter(IAdapter::Pointer adapter,
        const boost::property_tree::ptree & p)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::property_tree::ptree subtree;
    IBufferAdapter::Pointer buffer;
    std::set<std::string> devices;
    
    std::string type, name, signal;
    std::size_t index;
    
    if( !adapter )
    {
        throw std::runtime_error("Received an empty IAdapter::Pointer.");
    }

    buffer = boost::dynamic_pointer_cast<IBufferAdapter>(adapter);
    
    // i = 0 parses state information
    // i = 1 parses command information
    for( int i = 0; i < 2; i++ )
    {
        Logger.Debug << "Reading the " << (i == 0 ? "state" : "command")
                << " property tree specification." << std::endl;
        
        try
        {
            subtree = (i == 0 ? p.get_child("state") : p.get_child("command"));
        }
        catch( std::exception & e )
        {
            throw std::runtime_error("Failed to create adapter: "
                    + std::string(e.what()));
        }
        
        BOOST_FOREACH(boost::property_tree::ptree::value_type & child, subtree)
        {
            try
            {
                type    = child.second.get<std::string>("type");
                name    = child.second.get<std::string>("device");
                signal  = child.second.get<std::string>("signal");
                index   = child.second.get<std::size_t>("<xmlattr>.index");
            }
            catch( std::exception & e )
            {
                throw std::runtime_error("Failed to create adapter: "
                        + std::string(e.what()));
            }
            
            Logger.Debug << "At index " << index << " for the device signal ("
                    << name << "," << signal << ")." << std::endl;
            
            // create the device when first seen
            if( devices.count(name) == 0 )
            {
                CreateDevice(name, type, adapter);
                devices.insert(name);
            }

            if( buffer && i == 0 )
            {
                Logger.Debug << "Registering state info." << std::endl;
                buffer->RegisterStateInfo(name, signal, index);
            }
            else if( buffer && i == 1 )
            {
                Logger.Debug << "Registering command info." << std::endl;
                buffer->RegisterCommandInfo(name, signal, index);
            }
        }
    }
    Logger.Debug << "Initialized the device adapter." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Calls a registered function to create a new device of the given type.
///
/// @ErrorHandling Throws a std::runtime_error if type is not registered with
/// the factory.
/// @pre The type must be registered with CAdapterFactory::RegisterDeviceClass.
/// @post Redirects the call to the templated CAdapterFactory::CreateDevice.
/// @param name The unique identifier for the device to be created.
/// @param type The type of device to create.
/// @param adapter The adapter that will handle the data of the new device.
///
/// @limitations The device types must be registered prior to this call.
///////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::CreateDevice(std::string name, std::string type,
        IAdapter::Pointer adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( m_registry.count(type) == 0 )
    {
        throw std::runtime_error("Device type " + type + " is not registered.");
    }
    
    (this->*m_registry[type])(name, adapter);
}

} // namespace device
} // namespace freedm
} // namespace broker
