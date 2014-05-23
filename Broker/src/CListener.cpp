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
/// @fn CListener::CListener
/// @description Constructor
/// @pre An initialized socket is ready to be converted to a connection.
/// @post A new CConnection object is initialized.
///////////////////////////////////////////////////////////////////////////////
CListener::CListener()
    : m_socket(CBroker::Instance().GetIOService())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Access the singleton instance of the CListener
///////////////////////////////////////////////////////////////////////////////
CListener& CListener::Instance()
{
    static CListener listener;
    return listener;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CListener::Start
/// @description: Begin listening for incoming messages
/// @pre The object is initialized.
/// @post The connection is asynchronously waiting for messages.
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
/// Closes the listening socket.
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
/// @fn CListener::HandleRead
/// @description The callback which accepts messages from the remote sender.
/// @param e The errorcode if any associated.
/// @param bytes_transferred The size of the datagram being read.
/// @pre The connection has had start called and some message has been placed
///   in the buffer by the receive call.
/// @post The message has been delivered. This means that write connections
///   have been notified of ACK and standard messages have been redirected to
///   their appropriate places by the dispatcher. The incoming sequence number
///   for the source UUID has been incremented appropriately.
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
    ProtocolMessage pm;
    if(!pm.ParseFromArray(m_buffer.begin(), bytes_transferred))
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

    if(!IsValidPort(pm.source_port()))
    {
        Logger.Warn<<"Received message with invalid source port"<<pm.source_port()<<std::endl;
        ScheduleListen();
        return;
    }

    std::string uuid = pm.source_uuid();
    SRemoteHost host = { pm.source_hostname(), pm.source_port() };
    ///Make sure the hostname is registered:
    CConnectionManager::Instance().PutHost(uuid,host);
    ///Get the pointer to the connection:
    ConnectionPtr conn =
            CConnectionManager::Instance().GetConnectionByUUID(uuid);
    Logger.Debug<<"Fetched Connection"<<std::endl;

    if(pm.status() == ProtocolMessage::ACCEPTED)
    {
        Logger.Debug<<"Processing Accept Message"<<std::endl;
        Logger.Debug<<"Received ACK"<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
        conn->ReceiveACK(pm);
    }
    else if(conn->Receive(pm))
    {
        Logger.Debug<<"Accepted message "<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
        CDispatcher::Instance().HandleRequest(pm.module_message(), uuid);
    }
    else if(pm.status() != ProtocolMessage::CREATED)
    {
        Logger.Debug<<"Rejected message "<<pm.hash()<<":"<<pm.sequence_num()<<std::endl;
    }

    ScheduleListen();
}

///////////////////////////////////////////////////////////////////////////////
/// Schedule an asynchronous listen for the next incoming message. Our
/// invariant is that this function must be called after each HandleRead();
/// otherwise, we'll never receive new messages.
///////////////////////////////////////////////////////////////////////////////
void CListener::ScheduleListen()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug<<"Listening for next message"<<std::endl;
    // We don't care where the messages are coming from, but async_receive_from
    // requires that this variable remain valid until the handler is called.
    static boost::asio::ip::udp::endpoint ignored_sender_endpoint;
    m_socket.async_receive_from(
        boost::asio::buffer(m_buffer, CGlobalConfiguration::MAX_PACKET_SIZE),
        ignored_sender_endpoint, boost::bind(&CListener::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

    } // namespace broker
} // namespace freedm
