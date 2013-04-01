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
#include "CPnpAdapter.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalConfiguration.hpp"
#include "CFakeAdapter.hpp"
#include "PlugNPlayExceptions.hpp"

#include <utility>
#include <iostream>
#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/regex.hpp>
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
    : m_timeout(m_ios)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    RegisterDevices();
    m_thread = boost::thread(boost::bind(&CAdapterFactory::RunService, this));
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

void CAdapterFactory::StartSessionProtocol()
{
    CTcpServer::ConnectionHandler handler;
    unsigned short port;
    std::size_t size;

    size = m_ports.size();

    if( m_server )
    {
        throw std::logic_error("Session protocol already started.");
    }
    else if( size == 0 )
    {
        throw EDgiConfigError("No plug and play ports are specified.");
    }
    else
    {
        Logger.Info << size << " adapter port(s) available." << std::endl;

        // initialize the TCP variant of the session layer protocol
        port        = CGlobalConfiguration::instance().GetFactoryPort();
        handler     = boost::bind(&CAdapterFactory::StartSession, this);
        m_server    = CTcpServer::Create(m_ios, port,
                CGlobalConfiguration::instance().GetDevicesEndpoint() );
        m_server->RegisterHandler(handler);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Runs the i/o service with an infinite workload.
///
/// @pre None.
/// @post Runs m_ios and blocks the calling thread.
///
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::RunService()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::io_service::work runner(m_ios);

    try
    {
        m_ios.run();
    }
    catch (std::exception & e)
    {
        Logger.Fatal << "Fatal exception in the device ioservice: "
                << e.what() << std::endl;
        // FIXME We ought to pass the exception to the parent thread
        // and flush the broker's ioservice, but this is not possible without
        // Boost or C++11.
        std::exit(1);
    }

    Logger.Status << "Started the adapter i/o service." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new adapter and all of its devices.  The adapter is registered
/// with each device, and each device is registered with the global device
/// manager.  The adapter is configured to recognize its own device signals,
/// and started when the configuration is complete.
///
/// @ErrorHandling Throws an EDgiConfigError if the property tree is bad, or
/// EBadRequest if a PnP controller has assigned an unexpected signal to a
/// device (which would be an EDgiConfigError otherwise)
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
        throw EDgiConfigError("Failed to create adapter: "
                + std::string(e.what()));
    }
    
    Logger.Debug << "Building " << type << " adapter " << name << std::endl;
    
    // range check the properties
    if( name.empty() )
    {
        throw EDgiConfigError("Tried to create an unnamed adapter.");
    }
    else if( m_adapter.count(name) > 0 )
    {
        throw EDgiConfigError("Multiple adapters share the name: " + name);
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
    else if( type == "pnp" )
    {
        adapter = CPnpAdapter::Create(m_ios, subtree);
    }
    else if( type == "fake" )
    {
        adapter = CFakeAdapter::Create();
    }
    else
    {
        throw EDgiConfigError("Unregistered adapter type: " + type);
    }
    
    // store the adapter; note that InitializeAdapter can throw EBadRequest
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
    CPnpAdapter::Pointer pnp;
    
    if( m_adapter.count(identifier) == 0 )
    {
        throw std::runtime_error("No such adapter: " + identifier);
    }
    
    devices = m_adapter[identifier]->GetDevices();
    pnp = boost::dynamic_pointer_cast<CPnpAdapter>(m_adapter[identifier]);
    
    if( pnp )
    {
        Logger.Info << "Recycling an adapter port number." << std::endl;
        m_ports.insert(pnp->GetPortNumber());
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
    if( m_ports.insert(port).second )
    {
        Logger.Debug << "Added adapter port: " << port << std::endl;
    }
    else
    {
        Logger.Warn << "Duplicate adapter port: " << port << std::endl;
    }
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
/// @ErrorHandling Throws EDgiConfigError if the property tree has a bad
/// specification format. Could also throw EBadRequest if the adapter is a
/// CPnpAdapter and the Hello message assigns an unexpected signal to a device.
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
        throw std::logic_error("Received a null IAdapter::Pointer.");
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
            throw EDgiConfigError("Failed to create adapter: "
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
                throw EDgiConfigError("Failed to create adapter: "
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
                throw std::logic_error("Device " + name + " not in manager");
            }
            
            if( (i == 0 && !dev->HasStateSignal(signal)) ||
                (i == 1 && !dev->HasCommandSignal(signal)) )
            {
                std::string what = "Failed to create adapter: The "
                        + type + " device, " + name
                        + ", does not recognize the signal: " + signal;
                if (boost::dynamic_pointer_cast<CPnpAdapter>(adapter) != 0)
                {
                    throw EBadRequest(what);
                }
                else
                {
                    throw EDgiConfigError(what);
                }
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
/// @ErrorHandling Throws an EOutOfPorts if there are no available
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
        throw EOutOfPorts("No available port numbers for new adapter.");
    }
    
    port = *it;
    m_ports.erase(it);
    Logger.Debug << "Consumed the port number: " << port << std::endl;
    
    return port;
}

////////////////////////////////////////////////////////////////////////////////
/// Handles the 'Hello' message of the plug and play session protocol.
///
/// @pre m_socket must be connected to a remote endpoint.
/// @post Attempts to create a new adapter based on the hello message.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////

void CAdapterFactory::StartSession()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    Logger.Notice << "A wild client appears!" << std::endl;
    m_timeout.expires_from_now(boost::posix_time::seconds(2));
    m_timeout.async_wait(boost::bind(&CAdapterFactory::Timeout, this,
            boost::asio::placeholders::error));

    m_buffer.consume(m_buffer.size());
    boost::asio::async_read_until(m_server->GetSocket(), m_buffer, "\r\n\r\n",
            boost::bind(&CAdapterFactory::HandleRead, this,
            boost::asio::placeholders::error));
}

void CAdapterFactory::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if( !e ) 
    {
        m_server->GetSocket().close();
        Logger.Info << "Connection closed due to timeout." << std::endl;
        m_server->StartAccept();
    }
    else if( e == boost::asio::error::operation_aborted )
    {
        Logger.Debug << "Factory connection timeout aborted." << std::endl;
    }
    else
    {
        throw e;
    }
}

void CAdapterFactory::HandleRead(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        if( m_timeout.cancel() == 1 )
        {
            SessionProtocol();
        }
        else
        {
            Logger.Debug << "Dropped packet due to timeout." << std::endl;
        }
    }
    else
    {
        throw e;
    }
}

void CAdapterFactory::SessionProtocol()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
   
    std::istream packet(&m_buffer); 
    boost::asio::streambuf response;
    std::ostream response_stream(&response);
    
    boost::property_tree::ptree config;
    
    std::set<std::string> states, commands;
    std::string host, header, type, name, entry;
    int sindex = 1, cindex = 1;
    unsigned short port = 0;

    try
    {
        packet >> header >> host;
        //host = m_server->GetHostname();
        Logger.Info << "Received " << header << " from " << host << std::endl;
        
        if( header != "Hello" )
        {
            throw EBadRequest("Expected 'Hello' message: " + header);
        }
        if( m_adapter.count(host) > 0 )
        {
            throw EDuplicateSession("Duplicate session for " + host);
        }

        config.put("<xmlattr>.name", host);
        config.put("<xmlattr>.type", "pnp");
        config.put("info.identifier", host);
        // info.stateport handled below

        for( int i = 0; packet >> type >> name; i++ )
        {
            Logger.Debug << "Processing " << type << ":" << name << std::endl;
            
            if( m_prototype.count(type) == 0 )
            {
                throw EBadRequest("Unknown device type: " + type);
            }
            
            name = host + ":" + name;
            boost::replace_all(name, ".", ":");
            states = m_prototype[type]->GetStateSet();
            commands = m_prototype[type]->GetCommandSet();
            Logger.Debug << "Using adapter name " << name << std::endl;
            
            BOOST_FOREACH(std::string signal, states)
            {
                Logger.Debug << "Adding state for " << signal << std::endl;
                
                entry = name + signal;
                config.put("state." + entry + ".type", type);
                config.put("state." + entry + ".device", name);
                config.put("state." + entry + ".signal", signal);
                config.put("state." + entry + ".<xmlattr>.index", sindex);

                sindex++;
            }

            BOOST_FOREACH(std::string signal, commands)
            {
                Logger.Debug << "Adding command for " << signal << std::endl;
                
                entry = name + signal;
                config.put("command." + entry + ".type", type);
                config.put("command." + entry + ".device", name);
                config.put("command." + entry + ".signal", signal);
                config.put("command." + entry + ".<xmlattr>.index", cindex);

                cindex++;
            }
        }

        /*
        // remove me when done with error checking
        boost::property_tree::xml_writer_settings<char> settings('\t', 1);
        write_xml("file2.xml", config, std::locale(), settings);
        */

        while( true )
        {
            try
            {
                port = GetPortNumber();
                config.put("info.stateport", port);
                CreateAdapter(config);
                break;
            }
            catch(EBadRequest & e)
            {
                throw;
            }
            catch(EOutOfPorts & e)
            {
                throw;
            }
            catch(EDgiConfigError & e)
            {
                throw std::logic_error("Caught EDgiConfigError from "
                        "CAdapterFactory::CreateAdapter; note this makes no "
                        "sense for a plug and play adapter; what: "
                        + std::string(e.what()));
            }
            catch(std::exception & e)
            {
                // FIXME we can't rely on anything except the type of the
                // exception for control flow. This message could change
                // in the future and is definitely nonportable
                if( e.what() == std::string("Address already in use") )
                {
                    Logger.Warn << "Port already used: " << port << std::endl;
                    port = 0; // reset to default value
                }
                else
                {
                    throw;
                }
            }
        }
        
        response_stream << "Start\r\n";
        response_stream << "StatePort: " << port << "\r\n\r\n";
        Logger.Status << "Blocking to send Start to client" << std::endl;
    }
    catch(EBadRequest & e)
    {
        Logger.Notice << "Rejected client: " << e.what() << std::endl;

        if( port != 0 )
        {
            AddPortNumber(port);
        }
        
        response_stream << "BadRequest\r\n";
        response_stream << e.what() << "\r\n\r\n";

        Logger.Status << "Blocking to send BadRequest to client" << std::endl;
    }
    catch(EOutOfPorts & e)
    {
        Logger.Notice << "Rejected client: " << e.what() << std::endl;
        response_stream << "Error\r\nInsufficient adapter ports\r\n\r\n";
        Logger.Status << "Blocking to send Error to client" << std::endl;
    }
    catch(EDuplicateSession & e)
    {
        Logger.Notice << "Rejected client: " << e.what() << std::endl;
        // FIXME sending e.what() to the client is not robust
        // we need boost exceptions to handle this properly
        response_stream << "Error\r\n" << e.what() << "\r\n\r\n";
        Logger.Status << "Blocking to send Error to client" << std::endl;
    }
    
    try
    {
        boost::asio::write(m_server->GetSocket(), response);
    }
    catch(std::exception & e)
    {
        Logger.Warn << "Failed to respond to client: " << e.what() << std::endl; 
    }

    m_server->StartAccept();
}

} // namespace device
} // namespace freedm
} // namespace broker
