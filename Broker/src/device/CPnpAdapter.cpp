////////////////////////////////////////////////////////////////////////////////
/// @file           CPnpAdapter.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Adapter for plug-and-play devices.
///
/// @functions
///     CPnpAdapter::Create
///     CPnpAdapter::CPnpAdapter
///     CPnpAdapter::~CPnpAdapter
///     CPnpAdapter::Start
///     CPnpAdapter::Heartbeat
///     CPnpAdapter::GetPortNumber
///     CPnpAdapter::Timeout
///     CPnpAdapter::HandleRead
///     CPnpAdapter::AfterWrite
///     CPnpAdapter::Stop
///     CPnpAdapter::ReadStatePacket
///     CPnpAdapter::SendCommandPacket
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

#include "CPnpAdapter.hpp"
#include "CAdapterFactory.hpp"
#include "CLogger.hpp"
#include "CGlobalConfiguration.hpp"
#include "PlugNPlayExceptions.hpp"
#include "CTimings.hpp"
#include "SynchronousTimeout.hpp"

#include <map>
#include <sstream>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new shared instance of the PNP adapter.
///
/// @pre None.
/// @post Constructs a new PNP adapter.
/// @param service The i/o service for the internal TCP server.
/// @param p The property tree that specifies the adapter configuration.
/// @param client The TCP connection to use for this adapter.
/// @return shared_ptr to the new adapter.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IAdapter::Pointer CPnpAdapter::Create(boost::asio::io_service & service,
        boost::property_tree::ptree & p, CTcpServer::Connection client)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CPnpAdapter::Pointer(new CPnpAdapter(service, p, client));
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs a new PNP adapter.
///
/// @pre The ptree must have the 'identifier' and 'stateport' properties.
/// @post Creates a new TCP server on the specified 'stateport'.
/// @post Registers CPnpAdapter::HandleMessage with m_server.
/// @param service The i/o service for the TCP server.
/// @param p The property tree that configures the adapter.
/// @param client The TCP connection to use for this adapter.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CPnpAdapter::CPnpAdapter(boost::asio::io_service & service,
        boost::property_tree::ptree & p, CTcpServer::Connection client)
    : m_countdown(new boost::asio::deadline_timer(service))
    , m_ios(service)
    , m_client(client)
    , m_stopping(false)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_identifier = p.get<std::string>("identifier");
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor for the PNP adapter.
///
/// @pre None.
/// @post Destructs this object.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
CPnpAdapter::~CPnpAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Starts the internal countdown timer to destroy this object.
///
/// @pre None.
/// @post m_heartbeat set to call CPnpAdapter::Timeout on expiration.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    IBufferAdapter::Start();

    m_countdown->expires_from_now(boost::posix_time::milliseconds(
            CTimings::Get("DEV_PNP_HEARTBEAT")));
    m_countdown->async_wait(boost::bind(&CPnpAdapter::Timeout,
            shared_from_this(), boost::asio::placeholders::error));

    StartRead();
}

////////////////////////////////////////////////////////////////////////////////
/// Refreshes the heartbeat countdown timer.
///
/// @pre m_heartbeat must not have expired.
/// @post Resets the countdown of the m_heartbeat timer.
///
/// @limitations This call will do nothing if the timer has already expired.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::Heartbeat()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( m_countdown->expires_from_now(boost::posix_time::milliseconds(
            CTimings::Get("DEV_PNP_HEARTBEAT"))) != 0 )
    {
        Logger.Debug << "Reset an adapter heartbeat timer." << std::endl;
        m_countdown->async_wait(boost::bind(&CPnpAdapter::Timeout,
                shared_from_this(), boost::asio::placeholders::error));
    }
    else
    {
        Logger.Warn << "The heartbeat timer has already expired." << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Stops the adapter. Thread-safe.
///
/// @pre Adapter is started.
/// @post Adapter is stopped.
///
/// @limitations This is NOT the way to stop the plug and play protocol from
///              within the CPnpAdapter class.  From within CPnpAdapter, you
///              must instead call CAdapterFactory::RemoveAdapter, which
///              calls this function.  Otherwise, a reference to the adapter
///              will exist forever and its devices will not be properly
///              removed from the device manager.
///////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // The timer is not thread safe; it must be stopped from the device thread.
    // Note that this io_service may have already been stopped if the devices
    // thread threw an exception.  In this case, the cancel will never be
    // executed, but this is harmless.
    m_ios.post(boost::bind(&boost::asio::deadline_timer::cancel, m_countdown,
            boost::system::error_code()));

    {
        boost::lock_guard<boost::mutex> stoppingLock(m_stoppingMutex);
        m_stopping = true;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Stops the adapter due to timeout.
///
/// @pre None.
/// @post Calls CAdapterFactory::RemoveAdapter if the timer was not reset.
/// @param e The error code that signals if the timeout has been canceled.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        Logger.Status << "Removing an adapter due to timeout." << std::endl;

        try
        {
            std::string msg;
            msg = "Error\r\nConnection closed due to timeout.\r\n\r\n";
            TimedWrite(*m_client, boost::asio::buffer(msg),
                    CTimings::Get("DEV_SOCKET_TIMEOUT"));
        }
        catch(std::exception & e)
        {
            Logger.Info << "Failed to tell client about timeout." << std::endl;
        }

        CAdapterFactory::Instance().RemoveAdapter(m_identifier);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Schedules the next read from the plug and play device.
///
/// @pre m_client must not be uninitialized.
/// @post Clears the content of m_buffer prior to the read.
/// @post Calls CPnpAdapter::Heartbeat to refresh the connection.
/// @post Schedules the next socket connection on m_client.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::StartRead()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Heartbeat();
    m_buffer.consume(m_buffer.size());
    boost::asio::async_read_until(*m_client, m_buffer, "\r\n\r\n",
            boost::bind(&CPnpAdapter::HandleRead, shared_from_this(),
            boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////
/// Schedules the next write to the plug and play device.
///
/// @pre m_client must not be uninitialized.
/// @pre m_buffer must contain the data to write.
/// @post Calls CPnpAdapter::Heartbeat to refresh the connection.
/// @post Sends the content of m_buffer to m_client.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::StartWrite()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Heartbeat();

    boost::asio::async_write(*m_client, m_buffer,
            boost::bind(&CPnpAdapter::AfterWrite, shared_from_this(),
            boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////
/// Handles a packet received from the plug and play device.
///
/// @ErrorHandling If an exception occurs, it will be caught and a bad request
/// message will be sent to the client to indicate failure.
/// @pre The packet must be stored in m_buffer.
/// @post Processes the packet and prepares an appropriate response.
/// @param e The error code associated with the last read operation.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::HandleRead(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    {
        boost::lock_guard<boost::mutex> lock(m_stoppingMutex);
        if( m_stopping || e )
        {
            Logger.Debug << "HandleRead giving up : "
                << (m_stopping ? "received stop" : e.message()) << std::endl;
            return;
        }
    }

    std::istreambuf_iterator<char> end;
    std::iostream packet(&m_buffer);
    std::string data, header;

    try
    {
        Heartbeat();

        packet >> header;
        data = std::string(std::istreambuf_iterator<char>(packet), end);
        Logger.Debug << "Received " << header << " packet." << std::endl;

        m_buffer.consume(m_buffer.size());
        if( header == "DeviceStates" )
        {
            try
            {
                ReadStatePacket(data);
                if( m_buffer_initialized == false )
                {
                    RevealDevices();
                    m_buffer_initialized = true;
                }
                packet << GetCommandPacket();
            }
            catch(boost::bad_lexical_cast &)
            {
                std::string str = "received non-numeric value";
                Logger.Warn << "Corrupt state: " << str << std::endl;
                packet << "BadRequest\r\n" << str << "\r\n\r\n";
            }
            catch(EBadRequest & e)
            {
                Logger.Warn << "Corrupt state: " << e.what() << std::endl;
                packet << "BadRequest\r\n" << e.what() << "\r\n\r\n";
            }
        }
        else if( header == "PoliteDisconnect" )
        {
            Logger.Info << "Polite Disconnect Accepted" << std::endl;
            packet << "PoliteDisconnect\r\nAccepted\r\n\r\n";
            m_countdown->cancel();
            {
                boost::lock_guard<boost::mutex> lock(m_stoppingMutex);
                m_stopping = true;
            }
            CAdapterFactory::Instance().RemoveAdapter(m_identifier);
        }
        else
        {
            std::string msg = "Unknown header: " + header;
            packet << "BadRequest\r\n" << msg << "\r\n\r\n";
            Logger.Warn << msg << std::endl;
        }
        StartWrite();
    }
    catch(std::exception & e)
    {
        Logger.Info << m_identifier << " communication failed."
                << std::endl;
        Logger.Debug << "Reason: " << e.what() << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Prepares the next read operation after a successful write.
///
/// @pre None.
/// @post If the m_stopping flag has been raised, stops the adapter.
/// @post Otherwise, prepares the next read with CPnpAdapter::StartRead.
/// @param e The error code associated with the last write operation.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::AfterWrite(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::lock_guard<boost::mutex> lock(m_stoppingMutex);
    if( !m_stopping && !e )
    {
        Heartbeat();
        StartRead();
    }
    else
    {
        Logger.Debug << "AfterWrite giving up: "
                << (m_stopping ? "stop received" : e.message()) << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Processes the content of a state packet received from the device.
///
/// @ErrorHandling Throws a EBadRequest if the packet is malformed.
/// @pre The packet format must adhere to the session protocol specifications.
/// @post Extracts the device state information from packet.
/// @post Updates m_rxBuffer with the new state information.
/// @param packet The device packet that contains updated state information.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::ReadStatePacket(const std::string packet)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::map<std::size_t, SignalValue> temp;
    std::map<std::size_t, SignalValue>::iterator it, end;

    std::stringstream out;
    std::string name, signal, strval;

    std::size_t index;
    SignalValue value;

    Logger.Debug << "Processing packet: " << packet;

    out << packet;

    while( out >> name >> signal >> strval )
    {
        name = m_identifier + ":" + name;
        boost::replace_all(name, ".", ":");

        Logger.Debug << "Parsing: " << name << " " << signal << std::endl;

        DeviceSignal devsig(name, signal);
        std::string devsigstr = name + " " + signal;

        if( m_stateInfo.count(devsig) == 0 )
        {
            throw EBadRequest("Unknown device signal: " + devsigstr);
        }

        index = m_stateInfo[devsig];
        value = boost::lexical_cast<SignalValue>(strval);

        if( temp.insert(std::make_pair(index, value)).second == false )
        {
            throw EBadRequest("Duplicate device signal: " + devsigstr);
        }
    }

    if( temp.size() != m_rxBuffer.size() )
    {
        throw EBadRequest("Incomplete device state specification.");
    }

    // critical section
    {
        boost::unique_lock<boost::shared_mutex> lock(m_rxMutex);

        for( it = temp.begin(), end = temp.end(); it != end; it++ )
        {
            m_rxBuffer[it->first] = it->second;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Generates the next command packet from the current DGI commands.
///
/// @pre None.
/// @post Creates a command packet from the content of m_txBuffer.
/// @return A string that contains the next command packet.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
std::string CPnpAdapter::GetCommandPacket()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::map<const DeviceSignal, const std::size_t>::iterator it, end;
    std::stringstream packet;
    std::string devname, signal;
    SignalValue value;
    std::size_t index;

    packet << "DeviceCommands\r\n";

    boost::unique_lock<boost::shared_mutex> lock(m_txMutex);

    end = m_commandInfo.end();
    for( it = m_commandInfo.begin(); it != end; it++ )
    {
        devname = it->first.first;
        signal = it->first.second;

        // remove the hostname identifier
        index = devname.find_last_of(":");
        devname = devname.substr(index+1);

        value = m_txBuffer[it->second];

        packet << devname << " " << signal << " " << value << "\r\n";
    }
    Logger.Debug << "Sending packet:\n" << packet.str() << std::endl;
    packet << "\r\n";
    return packet.str();
}

} // namespace device
} // namespace broker
} // namespace freedm
