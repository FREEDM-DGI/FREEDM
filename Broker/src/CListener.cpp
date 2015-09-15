////////////////////////////////////////////////////////////////////////////////
/// @file         CListener.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a single CListener from a client.
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

#include "config.hpp"

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CGlobalConfiguration.hpp"
#include "CListener.hpp"
#include "CLogger.hpp"
#include "CClockSynchronizer.hpp"
#include "CConnection.hpp"
#include "messages/ModuleMessage.pb.h"
#include "messages/ProtocolMessage.pb.h"

#include <vector>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/system/error_code.hpp>

using boost::property_tree::ptree;

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CListener::CListener
/// @description Constructor. Creates the socket that will be used to listen
///     for incoming messages.
/// @pre None.
/// @post A socket is created using the Broker's io service.
///////////////////////////////////////////////////////////////////////////////
CListener::CListener()
    : m_socket(CBroker::Instance().GetIOService())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// CListener::Instance
/// @description Access the singleton instance of the CListener
///////////////////////////////////////////////////////////////////////////////
CListener& CListener::Instance()
{
    static CListener listener;
    return listener;
}

///////////////////////////////////////////////////////////////////////////////
/// CListener::Start
/// @description Causes the listener to start listening for new messages
/// @pre endpoint is a valid endpoint for the lister to listen on
/// @post The listener is not listening for incoming messages on the socket
///     bound to endpoint
/// @param endpoint the endpoint for the listener to listen on
///////////////////////////////////////////////////////////////////////////////
void CListener::Start(boost::asio::ip::udp::endpoint& endpoint)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_socket.open(endpoint.protocol());
    m_socket.bind(endpoint);
    ScheduleListen();
}

///////////////////////////////////////////////////////////////////////////////
/// CListener::Stop
/// @description Closes the listening socket.
/// @pre None
/// @post The socket used to listen for messages is closed.
///////////////////////////////////////////////////////////////////////////////
void CListener::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        m_socket.close();
    }
    catch (boost::system::system_error& e)
    {
        Logger.Error << "Error calling close: " << e.what() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CListener::HandleRead
/// @description The callback which accepts messages from the remote sender.
/// @param e The errorcode if any associated.
/// @param bytes_transferred The size of the datagram being read.
/// @pre The connection has had start called and some message has been placed
///   in the buffer by the receive call.
/// @post The message is scheduled for delivery by the dispatcher to one or
///     more modules. The message has been processed by the CConnection that
///     manages messages between this process and the sender. ScheduleListen()
///     is waiting for another datagram to arrive.
///////////////////////////////////////////////////////////////////////////////
void CListener::HandleRead(const boost::system::error_code& e,
                           std::size_t bytes_transferred)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if (e)
    {
        Logger.Error<<"HandleRead failed: " << e.message();
        ScheduleListen();
    }

    Logger.Debug<<"Loading protobuf"<<std::endl;
    ProtocolMessageWindow pmw;
    if(!pmw.ParseFromArray(m_buffer.begin(), bytes_transferred))
    {
        Logger.Error<<"Failed to load protobuf"<<std::endl;
        ScheduleListen();
        return;
    }

#ifdef CUSTOMNETWORK
    if((rand()%100) >= GetReliability())
    {
        Logger.Debug<<"Dropped datagram "<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
        ScheduleListen();
        return;
    }
#endif

    Logger.Debug<<"Fetching Connection"<<std::endl;
    std::string uuid = pmw.source_uuid();
    /// We can make the remote host from the endpoint:
    SRemoteHost host = { m_recv_from.address().to_string(), boost::lexical_cast<std::string>(m_recv_from.port()) };

    ///Make sure the hostname is registered:
    CConnectionManager::Instance().PutHost(uuid,host);

    ///Get the pointer to the connection:
    ConnectionPtr conn = CConnectionManager::Instance().CreateConnection(uuid, m_recv_from);
    //ConnectionPtr conn = CConnectionManager::Instance().GetConnectionByUUID(uuid);
    Logger.Debug<<"Fetched Connection"<<std::endl;

    BOOST_FOREACH(const ProtocolMessage &pm, pmw.messages())
    {
        if(pm.status() == ProtocolMessage::ACCEPTED)
        {
            Logger.Debug<<"Processing Accept Message"<<std::endl;
            Logger.Debug<<"Received ACK"<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
            conn->ReceiveACK(pm);
        }
        else if(conn->Receive(pm))
        {
            Logger.Debug<<"Accepted message "<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
            CDispatcher::Instance().HandleRequest(
                boost::make_shared<const ModuleMessage>(
                    ModuleMessage(pm.module_message())), uuid);
        }
        else if(pm.status() != ProtocolMessage::CREATED)
        {
            Logger.Debug<<"Rejected message "<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
        }
    }
    conn->OnReceive();
    ScheduleListen();
}

///////////////////////////////////////////////////////////////////////////////
/// CListener::ScheduleListen
/// @description Makes a call to the Broker's ioservice and requests that the
///     HandleRead function is called when a datagram arrives on the
///     Listener's socket.
/// @pre The m_socket is bound to an endpoint
/// @post HandleRead will be called when a datagram arrives at the socket
///////////////////////////////////////////////////////////////////////////////
void CListener::ScheduleListen()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug<<"Listening for next message"<<std::endl;
    // We don't care where the messages are coming from, but async_receive_from
    // requires that this variable remain valid until the handler is called.
    m_socket.async_receive_from(
        boost::asio::buffer(m_buffer, CGlobalConfiguration::MAX_PACKET_SIZE),
        m_recv_from, boost::bind(&CListener::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

    } // namespace broker
} // namespace freedm
