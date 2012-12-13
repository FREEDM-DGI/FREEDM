////////////////////////////////////////////////////////////////////////////////
/// @file           CAdapterFactory.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Handles the creation of device adapters.
///
/// @functions
///     CAdapterFactory::CAdapterFactory
///     CAdapterFactory::~CAdapterFactory
///     CAdapterFactory::Instance
///     CAdapterFactory::RunService
///     CAdapterFactory::CreateAdapter
///     CAdapterFactory::RemoveAdapter
///     CAdapterFactory::AddPortNumber
///     CAdapterFactory::CreateDevice
///     CAdapterFactory::InitializeAdapter
///     CAdapterFactory::GetPortNumber
///     CAdapterFactory::SessionProtocol
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

#include "CAdapterFactory.hpp"
#include "IBufferAdapter.hpp"
#include "CPscadAdapter.hpp"
#include "CRtdsAdapter.hpp"
#include "CArmAdapter.hpp"
#include "CGlobalConfiguration.hpp"

#include <sstream>
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

////////////////////////////////////////////////////////////////////////////////
/// Constructs an uninitialized factory.
///
/// @pre None.
/// @post Registers the recognized device classes.
/// @post Launches an i/o service on a separate thread.
/// @post Initializes the TCP server for the session protocol.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CAdapterFactory::CAdapterFactory()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    CTcpServer::ConnectionHandler handler;
    unsigned short port;
    
    RegisterDevices();
    
    // initialize the TCP variant of the session layer protocol
    port        = CGlobalConfiguration::instance().GetFactorySessionPort();
    handler     = boost::bind(&CAdapterFactory::SessionProtocol, this, _1);
    m_server    = CTcpServer::Create(m_ios, port);
    m_server->RegisterHandler(handler);

    m_thread = boost::thread(boost::bind(&CAdapterFactory::RunService, this));
}

////////////////////////////////////////////////////////////////////////////////
/// Stops the i/o service and waits for its thread to complete.
///
/// @pre None.
/// @post m_ios is stopped and the object is destroyed.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CAdapterFactory::~CAdapterFactory()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    m_ios.stop();
    Logger.Status << "Stopped the adapter i/o service." << std::endl;
    
    Logger.Notice << "Blocking until thread finishes execution." << std::endl;
    m_thread.join();
}

////////////////////////////////////////////////////////////////////////////////
/// Retrieves the singleton factory instance.
///
/// @pre None.
/// @post Creates a new factory on the first call.
/// @return The global instance of CAdapterFactory.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CAdapterFactory & CAdapterFactory::Instance()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    static CAdapterFactory instance;
    return instance;
}

////////////////////////////////////////////////////////////////////////////////
/// Runs the i/o service with an infinite workload.
///
/// @pre None.
/// @post Runs m_ios and blocks the calling thread.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::RunService()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::io_service::work runner(m_ios);
    m_ios.run();

    Logger.Status << "Started the adapter i/o service." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new adapter and all of its devices.  The adapter is registered
/// with each device, and each device is registered with the stored device
/// manager.  The adapter is configured to recognize its own device signals,
/// and started when the configuration is complete.
///
/// @ErrorHandling Throws a std::runtime_error if the property tree is bad.
/// @pre The adapter must not begin work until IAdapter::Start.
/// @pre The adapter's devices must not be specified in other adapters.
/// @post Calls CAdapterFactory::InitializeAdapter to create devices.
/// @post Starts the adapter through IAdapter::Start.
/// @param p The property tree that specifies a single adapter.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
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
        throw std::runtime_error("Failed to create adapter: " + std::string(e.what()));
    }
    
    Logger.Debug << "Building " << type << " adapter " << name << std::endl;
    
    // range check the properties
    if( name.empty() )
    {
        throw std::runtime_error("Tried to create an adapter without a name.");
    }
    else if( m_adapter.count(name) > 0 )
    {
        throw std::runtime_error("Multiple adapters share the name " + name);
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
    else if( type == "arm" )
    {
        adapter = CArmAdapter::Create(m_ios, subtree);
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

////////////////////////////////////////////////////////////////////////////////
/// Removes an adapter and all of its associated devices.
///
/// @ErrorHandling Throws a std::runtime_error if no such adapter exists.
/// @pre An adapter must exist with the provided identifier.
/// @post Removes the specified adapter from m_adapter.
/// @post Removes the adapter's devices from the device manager.
/// @param identifier The identifier of the adapter to remove.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::RemoveAdapter(const std::string identifier)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    std::set<std::string> devices;
    CArmAdapter::Pointer arm;
    
    if( m_adapter.count(identifier) == 0 )
    {
        throw std::runtime_error("Attempted to remove an adapter that does "
                + std::string("not exist: ") + identifier);
    }
    
    devices = m_adapter[identifier]->GetDevices();   
    arm = boost::dynamic_pointer_cast<CArmAdapter>(m_adapter[identifier]);
    if( arm )
    {
        Logger.Debug << "Making old port numbers available." << std::endl;
        m_ports.insert(arm->GetStatePort());
        m_ports.insert(arm->GetHeartbeatPort());
    }
    m_adapter.erase(identifier);
    
    BOOST_FOREACH(std::string device, devices)
    {
        CDeviceManager::Instance().RemoveDevice(device);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Adds a port number to the list of available TCP ports.
///
/// @pre None.
/// @post Adds the port number to m_ports.
/// @param port The port number to add to the available port set.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::AddPortNumber(const unsigned short port)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_ports.insert(port);
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new device and registers it with the device manager.
///
/// @ErrorHandling Throws a std::runtime_error if the name is already in use,
/// the type is not recognized, or the adapter is null.
/// @pre Type must be registered with CAdapterFactory::RegisterDevicePrototype.
/// @post Creates a new device using m_prototype[type].
/// @post Adds the new device to the device manager.
/// @param name The unique identifier for the device to be created.
/// @param type The string identifier for the type of device to create.
/// @param adapter The adapter that will handle the data of the new device.
///
/// @limitations The device types must be registered prior to this call.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::CreateDevice(const std::string name,
        const std::string type, IAdapter::Pointer adapter)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( CDeviceManager::Instance().DeviceExists(name) )
    {
        throw std::runtime_error("The device " + name + " already exists.");
    }
    
    if( m_prototype.count(type) == 0 )
    {
        throw std::runtime_error("Device type " + type + " is not registered.");
    }
    
    if( !adapter)
    {
        throw std::runtime_error("Attempted to create device with no adapter");
    }
    
    IDevice::Pointer device = m_prototype[type]->Create(name, adapter);
    CDeviceManager::Instance().AddDevice(device);
    
    Logger.Info << "Created new device: " << name << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
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
            throw std::runtime_error("Failed to create adapter: " + std::string(e.what()));
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
                adapter->RegisterDevice(name);
                devices.insert(name);
            }
            
            // check if the device recognizes the associated signal
            IDevice::Pointer dev = CDeviceManager::Instance().GetDevice(name);
            if( (i == 0 && !dev->HasStateSignal(signal)) ||
                (i == 1 && !dev->HasCommandSignal(signal)) )
            {
                throw std::runtime_error("Failed to create adapter: The "
                        + type + " device, " + name
                        + ", does not recognize the signal: " + signal);
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

////////////////////////////////////////////////////////////////////////////////
/// Retrieves and consumes the next available TCP port number.
///
/// @ErrorHandling Throws a std::runtime_exception if there are no available
/// port numbers in m_ports.
/// @pre m_ports must contain at least one unassigned element.
/// @post Removes and returns the first port number in m_ports.
/// @return The next available port number for a TCP server.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
unsigned short CAdapterFactory::GetPortNumber()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::set<unsigned short>::iterator it;
    unsigned short port;
    
    it = m_ports.begin();
    
    if( it == m_ports.end() )
    {
        std::runtime_error("The adapter factory does not have enough "
                + std::string("available port numbers."));
    }
    
    port = *it;
    m_ports.erase(it);
    Logger.Debug << "Consumed the port number: " << port << std::endl;
    
    return port;
}

////////////////////////////////////////////////////////////////////////////////
/// I HAVE BEEN COMMENTING ALL DAY - GIVE ME A BREAK.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::SessionProtocol(IServer::Pointer connection)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::property_tree::ptree adapter;
    std::set<std::string> states, commands;
    std::stringstream packet, response;
    std::string identifier, deviceType, deviceName, entry;
    unsigned short statePort, heartbeatPort;
    int stateIndex = 1, commandIndex = 1;
    
    Logger.Notice << "A wild client appears!" << std::endl;
    
    // receive hello
    packet << connection->ReceiveData();
    
    // get session information
    if( connection == m_server )
    {
        packet >> identifier >> identifier;
    }
    else
    {
        throw std::runtime_error("Received a null client connection.");
    }
    
    // check for duplicate session
    if( m_adapter.count(identifier) > 0 )
    {
        // is it possible for the adapter to be destroyed before between these?
        IAdapter::Pointer adapter = m_adapter[identifier];
        
        if( connection == m_server )
        {
            CArmAdapter::Pointer arm;
            
            arm = boost::dynamic_pointer_cast<CArmAdapter>(adapter);
            
            if( !arm )
            {
                throw std::runtime_error("Well this is embarassing.");
            }
            
            arm->Heartbeat();
            
            response << "StatePort: " << arm->GetStatePort() << "\r\n";
            response << "HeartbeatPort: " << arm->GetHeartbeatPort() << "\r\n";
            response << "\r\n";
        }
        else
        {
            throw std::runtime_error("Received a null client connection.");
        }
    }
    else
    {
        if( connection == m_server )
        {
            if( m_ports.size() < 2 )
            {
                throw std::runtime_error("Insufficient ports for new adapter.");
            }
            
            statePort = GetPortNumber();
            heartbeatPort = GetPortNumber();
            
            adapter.put("<xmlattr>.name", identifier);
            adapter.put("<xmlattr>.type", "arm");
            adapter.put("info.stateport", statePort);
            adapter.put("info.heartport", heartbeatPort);
            adapter.put("info.host", m_server->GetHostname());
            adapter.put("info.port", identifier);
            
            response << "StatePort: " << statePort << "\r\n";
            response << "HeartbeatPort: " << heartbeatPort << "\r\n";
            response << "\r\n";
        }
        else
        {
            throw std::runtime_error("Received a null client connection.");
        }

        // create the devices
        for( int i = 0; packet >> deviceType >> deviceName; i++ )
        {
            if( m_prototype.count(deviceType) == 0 )
            {
                throw std::runtime_error("Unrecognized type: " + deviceType);
            }
            
            deviceName = identifier + ":" + deviceName;
            states = m_prototype[deviceType]->GetStateSet();
            commands = m_prototype[deviceType]->GetCommandSet();

            BOOST_FOREACH(std::string signal, states)
            {
                entry = deviceName + signal;
                
                adapter.put("state." + entry + ".type", deviceType);
                adapter.put("state." + entry + ".device", deviceName);
                adapter.put("state." + entry + ".signal", signal);
                adapter.put("state." + entry + ".<xmlattr>.index", stateIndex);

                stateIndex++;
            }

            BOOST_FOREACH(std::string signal, commands)
            {
                entry = deviceName + signal;
                
                adapter.put("command." + entry + ".type", deviceType);
                adapter.put("command." + entry + ".device", deviceName);
                adapter.put("command." + entry + ".signal", signal);
                adapter.put("command." + entry + ".<xmlattr>.index", commandIndex);

                commandIndex++;
            }
        }
        
        CreateAdapter(adapter);
    }

    // send start
    if( connection == m_server )
    {
        CArmAdapter::Pointer arm;
        arm = boost::dynamic_pointer_cast<CArmAdapter>(m_adapter[identifier]);
        arm->Send(response.str());
    }
    else
    {
        throw std::runtime_error("tired of doing this");
    }
}

} // namespace device
} // namespace freedm
} // namespace broker
