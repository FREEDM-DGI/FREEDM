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
///     CAdapterFactory::Instance
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
#include "CDeviceManager.hpp"
#include "CGlobalConfiguration.hpp"

#include <utility>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

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
/// @post Registers the known device classes.
/// @post Initializes the session protocol TCP server.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CAdapterFactory::CAdapterFactory()
    : m_ios(CGlobalConfiguration::instance().GetService())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    CTcpServer::ConnectionHandler handler;
    unsigned short port;
    
    RegisterDevices();
    
    // initialize the TCP variant of the session layer protocol
    port        = CGlobalConfiguration::instance().GetFactoryPort();
    handler     = boost::bind(&CAdapterFactory::SessionProtocol, this);
    m_server    = CTcpServer::Create(m_ios, port);
    m_server->RegisterHandler(handler);
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
/// Creates a new adapter and all of its devices.  The adapter is registered
/// with each device, and each device is registered with the global device
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
        throw std::runtime_error("Failed to create adapter: "
                + std::string(e.what()));
    }
    
    Logger.Debug << "Building " << type << " adapter " << name << std::endl;
    
    // range check the properties
    if( name.empty() )
    {
        throw std::runtime_error("Tried to create an unnamed adapter.");
    }
    else if( m_adapter.count(name) > 0 )
    {
        throw std::runtime_error("Multiple adapters share the name: " + name);
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
        throw std::runtime_error("Unregistered adapter type: " + type);
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
        throw std::runtime_error("No such adapter: " + identifier);
    }
    
    devices = m_adapter[identifier]->GetDevices();
    arm = boost::dynamic_pointer_cast<CArmAdapter>(m_adapter[identifier]);
    
    if( arm )
    {
        Logger.Info << "Recycling an adapter port number." << std::endl;
        m_ports.insert(arm->GetPortNumber());
    }
    m_adapter.erase(identifier);
    
    Logger.Info << "Removed the adapter: " << identifier << std::endl;
    
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
        throw std::runtime_error("Unrecognized device type: " + type);
    }
    
    if( !adapter )
    {
        throw std::runtime_error("Tried to create device using null adapter.");
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
        throw std::runtime_error("Received a null IAdapter::Pointer.");
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
                adapter->RegisterDevice(name);
                devices.insert(name);
            }
            
            // check if the device recognizes the associated signal
            IDevice::Pointer dev = CDeviceManager::Instance().GetDevice(name);
            if( !dev )
            {
                throw std::runtime_error("Something bad happened.");
            }
            
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
        std::runtime_error("No available port numbers for new adapter.");
    }
    
    port = *it;
    m_ports.erase(it);
    Logger.Debug << "Consumed the port number: " << port << std::endl;
    
    return port;
}

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::SessionProtocol()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    boost::asio::streambuf packet;
    std::istream packet_stream(&packet);
    
    boost::asio::streambuf response;
    std::ostream response_stream(&response);
    
    boost::property_tree::ptree config;
    
    std::set<std::string> states, commands;
    std::string host, header, type, name, entry;
    int sindex = 1, cindex = 1;
    unsigned short port = 0;
    
    Logger.Notice << "A wild client appears!" << std::endl;
    
    try
    {
        Logger.Status << "Blocking for client hello message." << std::endl;
        boost::asio::read_until(m_server->GetSocket(), packet, "\r\n\r\n");
        
        host = m_server->GetHostname();
        
        packet_stream >> header;
        
        if( header != "Hello" )
        {
            Logger.Warn << "Unrecognized header: " << header << std::endl;
            throw std::runtime_error("Expected 'Hello' message");
        }
        
        if( m_adapter.count(host) > 0 )
        {
            Logger.Warn << "Duplicate session: " << host << std::endl;
            throw std::runtime_error("Duplicate session for " + host);
        }

        port = GetPortNumber();
        
        config.put("<xmlattr>.name", host);
        config.put("<xmlattr>.type", "arm");
        config.put("info.identifier", host);
        config.put("info.stateport", port);

        for( int i = 0; packet_stream >> type >> name; i++ )
        {
            if( m_prototype.count(type) == 0 )
            {
                Logger.Warn << "Unrecognized type: " << type << std::endl;
                throw std::runtime_error("Unknown device type: " + type);
            }
            
            name = host + ":" + name;
            for( int k = 0, n = name.size(); k < n; k++ )
            {
                if( name[k] == '.' )
                {
                    name[k] = ':';
                }
            }

            states = m_prototype[type]->GetStateSet();
            commands = m_prototype[type]->GetCommandSet();
            
            BOOST_FOREACH(std::string signal, states)
            {
                entry = name + signal;
                
                config.put("state." + entry + ".type", type);
                config.put("state." + entry + ".device", name);
                config.put("state." + entry + ".signal", signal);
                config.put("state." + entry + ".<xmlattr>.index", sindex);

                sindex++;
            }

            BOOST_FOREACH(std::string signal, commands)
            {
                entry = name + signal;
                
                config.put("command." + entry + ".type", type);
                config.put("command." + entry + ".device", name);
                config.put("command." + entry + ".signal", signal);
                config.put("command." + entry + ".<xmlattr>.index", cindex);

                cindex++;
            }
        }

        // remove me when done with error checking
        boost::property_tree::xml_writer_settings<char> settings('\t', 1);
        write_xml("file2.xml", config, std::locale(), settings);
        
        CreateAdapter(config);
        
        response_stream << "Start\r\n";
        response_stream << "StatePort: " << port << "\r\n\r\n";
    }
    catch(std::exception & e)
    {
        Logger.Notice << "Rejected client due to error." << std::endl;
        
        if( port != 0 )
        {
            AddPortNumber(port);
        }
        
        response_stream << "BadRequest\r\n";
        response_stream << e.what() << "\r\n\r\n";
    }
    
    try
    {
        Logger.Status << "Blocking for client start message." << std::endl;
        boost::asio::write(m_server->GetSocket(), response);
    }
    catch(std::exception & e)
    {
        Logger.Warn << "Failed to respond to client: " << e.what() << std::endl; 
    }
}

} // namespace device
} // namespace freedm
} // namespace broker
