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
///     CPnpAdapter::HandleMessage
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

#include <map>
#include <sstream>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
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
    : m_countdown(service)
    , m_client(client)
    , m_stop(false)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_identifier = p.get<std::string>("identifier");
    StartRead();
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

    // TODO: this fella should be configurable
    m_countdown.expires_from_now(boost::posix_time::seconds(5));
    m_countdown.async_wait(boost::bind(&CPnpAdapter::Timeout, this, _1));
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

    // TODO: hello configurable option
    if( m_countdown.expires_from_now(boost::posix_time::seconds(5)) != 0 )
    {
        Logger.Debug << "Reset an adapter heartbeat timer." << std::endl;
        m_countdown.async_wait(boost::bind(&CPnpAdapter::Timeout, this, _1));
    }
    else
    {
        Logger.Warn << "The heartbeat timer has already expired." << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Attempts to destroy the adapter due to timeout.
///
/// @pre None.
/// @post Calls CAdapterFactory::RemoveAdapter if the timer was not reset.
/// @param e The error code that signals if the timeout has been canceled.
///
/// @limitations This function will not work as intended if the shared pointer
/// to the calling object has a reference outside of CAdapterFactory.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::Timeout(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !e )
    {
        Logger.Status << "Removing an adapter due to timeout." << std::endl;
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
            boost::bind(&CPnpAdapter::HandleRead, this,
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
            boost::bind(&CPnpAdapter::HandleWrite, this,
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
    if( !e )
    {
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
                Logger.Debug << "Polite Disconnect Accepted" << std::endl;
                packet << "PoliteDisconnect\r\nAccepted\r\n\r\n";
                m_stop = true;
            }
            else
            {
                Logger.Warn << "Unknown header: " << header << std::endl;
                packet << "BadRequest\r\n\r\n";
                m_stop = true;
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
}

////////////////////////////////////////////////////////////////////////////////
/// Prepares the next read operation after a successful write.
///
/// @pre None.
/// @post If the m_stop flag has been raised, stops the adapter.
/// @post Otherwise, prepares the next read with CPnpAdapter::StartRead.
/// @param e The error code associated with the last write operation.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
void CPnpAdapter::HandleWrite(const boost::system::error_code & e)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if( m_stop )
    {
	    boost::system::error_code null;

        m_countdown.cancel();
        Timeout(null);
    }
    else if( !e )
    {
        Heartbeat();
        StartRead();
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
