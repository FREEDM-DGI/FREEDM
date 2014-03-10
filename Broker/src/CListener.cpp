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

#include "CBroker.hpp"
#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CGlobalConfiguration.hpp"
#include "CListener.hpp"
#include "CLogger.hpp"
#include "CMessage.hpp"
#include "config.hpp"
#include "CClockSynchronizer.hpp"

#include <vector>

#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>

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
    m_socket.async_receive_from(boost::asio::buffer(m_buffer, CGlobalConfiguration::MAX_PACKET_SIZE),
            m_endpoint, boost::bind(&CListener::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
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
    if (!e)
    {
        /// I'm removing request parser because it is an appalling heap of junk
        std::stringstream iss;
        std::ostreambuf_iterator<char> iss_it(iss);
        std::copy(m_buffer.begin(), m_buffer.begin()+bytes_transferred, iss_it);
        // Create a new CMessage pointer
        m_message = MessagePtr(new CMessage);

        try
        {
            Logger.Debug<<"Loading xml:"<<std::endl;
            m_message->Load(iss);
        }
        catch(std::exception &e)
        {
            Logger.Error<<"Couldn't parse message XML: "<<e.what()<<std::endl;
            m_socket.async_receive_from(boost::asio::buffer(m_buffer, CGlobalConfiguration::MAX_PACKET_SIZE),
                m_endpoint, boost::bind(&CListener::HandleRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
            return;
        }

        std::string uuid = m_message->GetSourceUUID();
        SRemoteHost hostname = m_message->GetSourceHostname();
        ///Make sure the hostname is registered:
        CConnectionManager::Instance().PutHost(uuid,hostname);
        ///Get the pointer to the connection:
        CConnection::ConnectionPtr conn;
        conn = CConnectionManager::Instance().GetConnectionByUUID(uuid);
        Logger.Debug<<"Fetched Connection"<<std::endl;
#ifdef CUSTOMNETWORK
        if((rand()%100) >= GetReliability())
        {
            Logger.Debug<<"Dropped datagram "<<m_message->GetHash()<<":"
                          <<m_message->GetSequenceNumber()<<std::endl;
            goto listen;
        }
#endif
        if(m_message->GetStatus() == freedm::broker::CMessage::Accepted)
        {
            Logger.Debug<<"Processing Accept Message"<<std::endl;
            ptree pp = m_message->GetProtocolProperties();
            size_t hash = pp.get<size_t>("src.hash");
            Logger.Debug<<"Received ACK"<<hash<<":"
                            <<m_message->GetSequenceNumber()<<std::endl;
            conn->ReceiveACK(*m_message);
        }
        else if(m_message->GetStatus() == freedm::broker::CMessage::ClockReading && conn->Receive(*m_message))
        {
            Logger.Debug<<"Got A clock message"<<std::endl;
            CBroker::Instance().GetClockSynchronizer().HandleRead(m_message);
        }
        else if(conn->Receive(*m_message))
        {
            Logger.Debug<<"Accepted message "<<m_message->GetHash()<<":"
                          <<m_message->GetSequenceNumber()<<std::endl;
            CDispatcher::Instance().HandleRequest(m_message);
        }
        else if(m_message->GetStatus() != freedm::broker::CMessage::Created)
        {
            Logger.Debug<<"Rejected message "<<m_message->GetHash()<<":"
                          <<m_message->GetSequenceNumber()<<std::endl;
        }
#ifdef CUSTOMNETWORK
listen:
#endif
        Logger.Debug<<"Listening for next message"<<std::endl;
        m_socket.async_receive_from(boost::asio::buffer(m_buffer, CGlobalConfiguration::MAX_PACKET_SIZE),
                m_endpoint, boost::bind(&CListener::HandleRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

    } // namespace broker
} // namespace freedm
