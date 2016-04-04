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
///     CAdapterFactory::RunService
///     CAdapterFactory::Stop
///     CAdapterFactory::CreateAdapter
///     CAdapterFactory::RemoveAdapter
///     CAdapterFactory::InitializeAdapter
///     CAdapterFactory::CreateDevice
///     CAdapterFactory::StartSessionProtocol
///     CAdapterFactory::StartSession
///     CAdapterFactory::HandleRead
///     CAdapterFactory::Timeout
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
#include "CRtdsAdapter.hpp"
#include "CPnpAdapter.hpp"
#include "CDeviceManager.hpp"
#include "CGlobalConfiguration.hpp"
#include "CFakeAdapter.hpp"
#include "PlugNPlayExceptions.hpp"
#include "SynchronousTimeout.hpp"
#include "CTimings.hpp"

#include <utility>
#include <iostream>
#include <map>
#include <set>

#include <signal.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/system/system_error.hpp>
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

    std::string deviceCfgFile =
        CGlobalConfiguration::Instance().GetDeviceConfigPath();

    if( deviceCfgFile.empty() )
    {
        Logger.Status << "System will start no device classes." << std::endl;
    }
    else
    {
        m_builder = CDeviceBuilder(deviceCfgFile);
    }

    unsigned short factoryPort =
        CGlobalConfiguration::Instance().GetFactoryPort();

    if( factoryPort )
    {
        Logger.Status << "Plug and play devices enabled." << std::endl;
        StartSessionProtocol(factoryPort);
    }
    else
    {
        Logger.Status << "Plug and play devices disabled." << std::endl;
    }

    std::string adapterCfgFile =
        CGlobalConfiguration::Instance().GetAdapterConfigPath();

    if( adapterCfgFile.empty() )
    {
        Logger.Status << "System will start without adapters." << std::endl;
    }
    else
    {
        Logger.Status << "Using devices in " << adapterCfgFile << std::endl;

        try
        {
            boost::property_tree::ptree adapterList;
            boost::property_tree::read_xml(adapterCfgFile, adapterList);

            BOOST_FOREACH(boost::property_tree::ptree::value_type & t,
                    adapterList.get_child("root"))
            {
                CreateAdapter(t.second);
            }
        }
        catch(boost::property_tree::xml_parser_error & e)
        {
            throw std::runtime_error("Failed to create device adapters: "
                    + std::string(e.what()));
        }
        catch(std::exception & e)
        {
            throw std::runtime_error(adapterCfgFile+": "+e.what());
        }
    }

    // Last because we don't want this thread to run if construction fails.
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

    boost::asio::io_service::work workload(m_ios);

    try
    {
        Logger.Status << "Starting the adapter i/o service." << std::endl;
        m_ios.run();
    }
    catch (std::exception & e)
    {
        Logger.Fatal << "Fatal exception in the device ioservice: "
                << e.what() << std::endl;
        // The Broker will stop us.
        raise(SIGTERM);
    }

    Logger.Status << "The adapter i/o service has stopped." << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Stops the i/o service and removes all devices from the device manager.
/// This function must be called from outside the devices thread.
///
/// @pre None
/// @post All the devices of every adapter in the system are removed.
/// @post The IOService has stopped.
/// @post The devices thread is detatched and stopped (unless called from it).
/// @ErrorHandling Guaranteed not to throw. Errors are only logged.
/// @limitations MUST be called from outside the devices thread.
///////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    assert(boost::this_thread::get_id() != m_thread.get_id());

    try
    {
        if (m_server)
        {
            m_server->Stop();
        }

        // Remove every adapter without using an invalid iterator
        for (std::map<std::string, IAdapter::Pointer>::iterator i =
                    m_adapters.begin();
             i != m_adapters.end(); i = m_adapters.begin())
        {
            RemoveAdapter(i->first);
        }

        m_ios.stop();
        m_thread.join();
    }
    catch (std::exception & e)
    {
        Logger.Error << "Caught exception when stopping AdapterFactory: "
                << e.what() << std::endl;
    }
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
    else if( m_adapters.count(name) > 0 )
    {
        throw EDgiConfigError("Multiple adapters share the name: " + name);
    }

    // create the adapter
    // FIXME - use plugins or something, this sucks
    if( type == "rtds" )
    {
        adapter = CRtdsAdapter::Create(m_ios, subtree);
    }
    else if( type == "pnp" )
    {
        adapter = CPnpAdapter::Create(m_ios, subtree, m_server->GetClient());
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
    m_adapters[name] = adapter;
    Logger.Info << "Created the " << type << " adapter " << name << std::endl;

    // signal construction complete
    adapter->Start();
}

////////////////////////////////////////////////////////////////////////////////
/// Removes an adapter and all of its associated devices.
///
/// @ErrorHandling Throws a std::runtime_error if no such adapter exists.
/// @pre An adapter must exist with the provided identifier.
/// @post Removes the specified adapter from m_adapters.
/// @post Removes the adapter's devices from the device manager.
/// @param identifier The identifier of the adapter to remove.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::RemoveAdapter(const std::string identifier)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::set<std::string> devices;

    if( m_adapters.count(identifier) == 0 )
    {
        throw std::runtime_error("No such adapter: " + identifier);
    }

    devices = m_adapters[identifier]->GetDevices();

    m_adapters[identifier]->Stop();
    m_adapters.erase(identifier);
    Logger.Info << "Removed the adapter: " << identifier << std::endl;

    BOOST_FOREACH(std::string device, devices)
    {
        CDeviceManager::Instance().RemoveDevice(device);
    }
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
    boost::optional<SignalValue> value;
    CDevice::Pointer device;

    std::map<std::string, std::string> devtype;
    std::map<std::string, unsigned int> states;
    std::map<std::string, unsigned int> commands;
    std::map<std::string, unsigned int>::iterator it;

    std::string type, name, signal;
    std::size_t index;

    if( !adapter )
    {
        throw std::logic_error("Received a null IAdapter::Pointer.");
    }
    if( p.count("state") > 1 )
    {
        throw EDgiConfigError("XML contains multiple state tags");
    }
    if( p.count("command") > 1 )
    {
        throw EDgiConfigError("XML contains multiple command tags");
    }

    IBufferAdapter::Pointer buffer =
            boost::dynamic_pointer_cast<IBufferAdapter>(adapter);
    CFakeAdapter::Pointer fake =
            boost::dynamic_pointer_cast<CFakeAdapter>(adapter);

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
                value   = child.second.get_optional<SignalValue>("value");
                index   = child.second.get<std::size_t>("<xmlattr>.index");

                if( child.second.size() != 4 && !fake )
                {
                    std::stringstream ss;
                    ss << "Invalid entry at " << (i == 0 ? "state" : "command")
                            << " index = " << index << ": too many subtags";
                    throw std::runtime_error(ss.str());
                }
            }
            catch( std::exception & e )
            {
                throw EDgiConfigError("Failed to create adapter: "
                        + std::string(e.what()));
            }

            Logger.Debug << "At index " << index << " for the device signal ("
                    << name << "," << signal << ")." << std::endl;

            // create the device when first seen
            if( devtype.count(name) == 0 )
            {
                CreateDevice(name, type, adapter);
                adapter->RegisterDevice(name);
                devtype[name]  = type;
                states[name]   = 0;
                commands[name] = 0;
            }

            if( devtype[name] != type )
            {
                std::string what = "Failed to create adapter: Multiple "
                        + std::string("devices share the name: ") + name;
                throw EDgiConfigError(what);
            }

            // check if the device recognizes the associated signal
            device = CDeviceManager::Instance().m_hidden_devices.at(name);

            if( i == 0 && device->HasState(signal) )
            {
                ++states[name];
            }
            else if( i == 1 && device->HasCommand(signal) )
            {
                ++commands[name];
            }
            else
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
            else if( fake && value )
            {
                SignalValue oldval = fake->GetState(name, signal);
                if( oldval != SignalValue() && oldval != value.get() )
                {
                    throw std::runtime_error("Duplicate Initial Value");
                }
                fake->SetCommand(name, signal, value.get());
            }
        }
    }

    for( it = states.begin(); it != states.end(); it++ )
    {
        device = CDeviceManager::Instance().m_hidden_devices.at(it->first);

        if( device->GetStateSet().size() != it->second )
        {
            std::string what = "Failed to create adapter: The device "
                    + it->first + " is missing at least one state.";
            if (boost::dynamic_pointer_cast<CPnpAdapter>(adapter) != 0)
            {
                throw EBadRequest(what);
            }
            else
            {
                throw EDgiConfigError(what);
            }
        }
    }
    for( it = commands.begin(); it != commands.end(); it++ )
    {
        device = CDeviceManager::Instance().m_hidden_devices.at(it->first);

        if( device->GetCommandSet().size() != it->second )
        {
            std::string what = "Failed to create adapter: The device "
                    + it->first + " is missing at least one command.";
            if (boost::dynamic_pointer_cast<CPnpAdapter>(adapter) != 0)
            {
                throw EBadRequest(what);
            }
            else
            {
                throw EDgiConfigError(what);
            }
        }
    }

    Logger.Debug << "Initialized the device adapter." << std::endl;
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

    if( !adapter )
    {
        throw std::runtime_error("Tried to create device using null adapter.");
    }

    CDevice::Pointer device = m_builder.CreateDevice(name, type, adapter);
    CDeviceManager::Instance().AddDevice(device);

    Logger.Info << "Created new device: " << name << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Initializes the plug and play session protocol.
///
/// @param port the port over which to run the session protocol
///
/// @ErrorHandling Throws a std::logic_error if the session protoocl has been
/// initialized through a prior call to this function.
/// @pre m_server must not be initialized by a prior call to this function.
/// @post m_server is created to accept connections from plug and play devices.
///
/// @limitations This function must be called at most once.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::StartSessionProtocol(unsigned short port)
{
    CTcpServer::ConnectionHandler handler;

    if( m_server )
    {
        throw std::logic_error("Session protocol already started.");
    }
    else
    {
        // initialize the TCP variant of the session layer protocol
        handler     = boost::bind(&CAdapterFactory::StartSession, this);
        m_server    = CTcpServer::Create(m_ios, port,
                CGlobalConfiguration::Instance().GetDevicesEndpoint() );
        m_server->RegisterHandler(handler);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Prepares to read the hello message from a new plug and play device.
///
/// @pre m_server must be connected to a remote endpoint.
/// @post m_timeout is started to disconnect the device if it does not respond.
/// @post Schedules a read into m_buffer from the current m_server connection.
///
/// @limitations This function must only be called by m_server.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::StartSession()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    Logger.Notice << "A wild client appears!" << std::endl;
    m_timeout.expires_from_now(boost::posix_time::seconds(CTimings::Get("DEV_PNP_HEARTBEAT")));
    m_timeout.async_wait(boost::bind(&CAdapterFactory::Timeout, this,
            boost::asio::placeholders::error));

    m_buffer.consume(m_buffer.size());
    boost::asio::async_read_until(*m_server->GetClient(), m_buffer, "\r\n\r\n",
            boost::bind(&CAdapterFactory::HandleRead, this,
            boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////
/// Starts the session protocol after a successful read from a device.
///
/// @pre None.
/// @post If a successful read, calls CAdapterFactory::SessionProtocol.
/// @param e The error code associated with the last read operation.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
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
            Logger.Notice << "Dropped packet due to timeout." << std::endl;
        }
    }
    else if( e == boost::asio::error::operation_aborted )
    {
        Logger.Notice << "Controller failed to send valid Hello." << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Closes a plug and play connection if it does not send a well-formed packet.
///
/// @pre None.
/// @post If timeout or error, closes the current m_server connection.
/// @param e The error code associated with the timer.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CAdapterFactory::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        Logger.Notice << "Connection closed due to timeout." << std::endl;

        try
        {
            std::string msg;
            msg = "Error\r\nConnection closed due to timeout.\r\n\r\n";
            TimedWrite(*m_server->GetClient(), boost::asio::buffer(msg),
                    CTimings::Get("DEV_SOCKET_TIMEOUT"));
        }
        catch(std::exception & e)
        {
            Logger.Info << "Failed to tell client about timeout." << std::endl;
        }

        m_server->GetClient()->cancel();
        m_server->StartAccept();
    }
    else if( e == boost::asio::error::operation_aborted )
    {
        // Timeout was cancelled. Hopefully a good Hello was received!
    }
    else
    {
        Logger.Warn << "Connection closed: " << e.message() << std::endl;
        m_server->GetClient()->cancel();
        m_server->StartAccept();
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Handles the hello message for the plug and play session protoocl.
///
/// @pre m_buffer must contain the device hello packet.
/// @post If the packet is well-formed, creates a new adapter and responds to
/// the plug and play connection with a start packet.
/// @post Otherwise, responds with a bad request that indicates the error.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
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

    try
    {
        packet >> header >> host;
        Logger.Info << "Received " << header << " from " << host << std::endl;

        if( header != "Hello" )
        {
            throw EBadRequest("Expected 'Hello' message: " + header);
        }
        if( m_adapters.count(host) > 0 )
        {
            throw EDuplicateSession("Duplicate session for " + host);
        }

////////////////////////////////////////////////////////////////////////////////
/// Reformat the packet as a property tree that can be used with CreateAdapter.
////////////////////////////////////////////////////////////////////////////////
        config.put("<xmlattr>.name", host);
        config.put("<xmlattr>.type", "pnp");
        config.put("info.identifier", host);
        config.put("state", "");
        config.put("command", "");

        for( int i = 0; packet >> type >> name; i++ )
        {
            Logger.Debug << "Processing " << type << ":" << name << std::endl;

            try
            {
                DeviceInfo info = m_builder.GetDeviceInfo(type);
                states = info.s_state;
                commands = info.s_command;
            }
            catch(std::exception & e)
            {
                throw EBadRequest("Unknown device type: " + type);
            }

            name = host + ":" + name;
            boost::replace_all(name, ".", ":");
            Logger.Debug << "Using adapter name " << name << std::endl;

            BOOST_FOREACH(std::string signal, states)
            {
                Logger.Debug << "Adding state for " << signal << std::endl;

                boost::property_tree::ptree temp;
                temp.put("type", type);
                temp.put("device", name);
                temp.put("signal", signal);
                temp.put("<xmlattr>.index", sindex);

                entry = name + signal;
                config.add_child("state." + entry, temp);

                sindex++;
            }

            BOOST_FOREACH(std::string signal, commands)
            {
                Logger.Debug << "Adding command for " << signal << std::endl;

                boost::property_tree::ptree temp;
                temp.put("type", type);
                temp.put("device", name);
                temp.put("signal", signal);
                temp.put("<xmlattr>.index", cindex);

                entry = name + signal;
                config.add_child("command." + entry, temp);

                cindex++;
            }
        }
////////////////////////////////////////////////////////////////////////////////
/// The config property tree now contains a valid adapter specification.
////////////////////////////////////////////////////////////////////////////////

        try
        {
            CreateAdapter(config);
        }
        catch(EDgiConfigError & e)
        {
            throw std::logic_error("Caught EDgiConfigError from "
                    "CAdapterFactory::CreateAdapter; note this makes no "
                    "sense for a plug and play adapter; what: "
                    + std::string(e.what()));
        }

        response_stream << "Start\r\n\r\n";
        Logger.Status << "Blocking to send Start to client" << std::endl;
    }
    catch(EBadRequest & e)
    {
        Logger.Warn << "Rejected client: " << e.what() << std::endl;

        response_stream << "BadRequest\r\n";
        response_stream << e.what() << "\r\n\r\n";

        Logger.Status << "Blocking to send BadRequest to client" << std::endl;
    }
    catch(std::exception & e)
    {
        Logger.Warn << "Rejected client: " << e.what() << std::endl;
        response_stream << "Error\r\n" << e.what() << "\r\n\r\n";
        Logger.Status << "Blocking to send Error to client" << std::endl;
    }

    try
    {
        TimedWrite(*m_server->GetClient(), response,
                CTimings::Get("DEV_SOCKET_TIMEOUT"));
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
