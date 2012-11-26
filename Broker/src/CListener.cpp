////////////////////////////////////////////////////////////////////////////////
/// @file         CListener.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
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

#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
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
/// @description Constructor for the CConnection object. Since the change to
///   udp, this object can act either as a listener or sender (but not both)
///   to have the object behave as a listener, Start() should be called on it.
/// @pre An initialized socket is ready to be converted to a connection.
/// @post A new CConnection object is initialized.
/// @param p_ioService The socket to use for the connection.
/// @param p_manager The related connection manager that tracks this object.
/// @param p_dispatch The dispatcher responsible for applying read/write 
///   handlers to messages.
/// @param uuid: The uuid this node connects to, or what listener.
///////////////////////////////////////////////////////////////////////////////
CListener::CListener(boost::asio::io_service& p_ioService,
  CConnectionManager& p_manager, CBroker& p_broker, std::string uuid)
  : CReliableConnection(p_ioService,p_manager,p_broker,uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CListener::Start
/// @description: Starts the receive routine which causes this socket to behave
///   as a listener.
/// @pre The object is initialized.
/// @post The connection is asynchronously waiting for messages.
///////////////////////////////////////////////////////////////////////////////

void CListener::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    GetSocket().async_receive_from(boost::asio::buffer(m_buffer, CReliableConnection::MAX_PACKET_SIZE),
            m_endpoint, boost::bind(&CListener::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CListener::Stop
/// @description Stops the socket and cancels the timeout timer. Does not
///   need to be called on a listening connection (ie one that has had
///   Start() called on it.
/// @pre Any initialized CConnection object.
/// @post The underlying socket is closed and the message timeout timer is
///        cancelled.
///////////////////////////////////////////////////////////////////////////////
void CListener::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}
 
///////////////////////////////////////////////////////////////////////////////
/// @fn CListener::HandleRead
/// @description The callback which accepts messages from the remote sender.
/// @param e The errorcode if any associated.
/// @param bytes_transferred The size of the datagram being read.
/// @pre The connection has had start called and some message has been placed
///   in the buffer by the recieve call.
/// @post The message has been delivered. This means that write connections
///   have been notified of ACK and standard messages have been redirected to
///   their appropriate places by the dispatcher. The incoming sequence number
///   for the source UUID has been incremented appropriately.
///////////////////////////////////////////////////////////////////////////////
#pragma GCC diagnostic ignored "-Wunused-label"
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
        
        try
        {
            Logger.Debug<<"Loading xml:"<<std::endl;
            m_message.Load(iss);
        }
        catch(std::exception &e)
        {
            Logger.Error<<"Couldn't parse message XML: "<<e.what()<<std::endl;
            GetSocket().async_receive_from(boost::asio::buffer(m_buffer, CReliableConnection::MAX_PACKET_SIZE),
                m_endpoint, boost::bind(&CListener::HandleRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
            return;
        }

        ptree x = static_cast<ptree>(m_message);
        std::string uuid = m_message.GetSourceUUID();
        SRemoteHost hostname = m_message.GetSourceHostname();
        ///Make sure the hostname is registered:
        GetConnectionManager().PutHostname(uuid,hostname);                        
        ///Get the pointer to the connection:
        CConnection::ConnectionPtr conn;
        conn = GetConnectionManager().GetConnectionByUUID(uuid);
        Logger.Debug<<"Fetched Connection"<<std::endl;
#ifdef CUSTOMNETWORK
        if((rand()%100) >= GetReliability())
        {
            Logger.Debug<<"Dropped datagram "<<m_message.GetHash()<<":"
                          <<m_message.GetSequenceNumber()<<std::endl;
            goto listen;
        }
#endif
        if(m_message.GetStatus() == freedm::broker::CMessage::Accepted)
        {
            Logger.Debug<<"Processing Accept Message"<<std::endl;
            ptree pp = m_message.GetProtocolProperties();
            size_t hash = pp.get<size_t>("src.hash");
            Logger.Debug<<"Recieved ACK"<<hash<<":"
                            <<m_message.GetSequenceNumber()<<std::endl;
            conn->RecieveACK(m_message);
        }
        else if(m_message.GetStatus() == freedm::broker::CMessage::ClockReading && conn->Recieve(m_message))
        {
            Logger.Debug<<"Got A clock message"<<std::endl;
            GetBroker().GetClockSynchronizer().HandleRead(m_message);
        }
        else if(conn->Recieve(m_message))
        {
            Logger.Debug<<"Accepted message "<<m_message.GetHash()<<":"
                          <<m_message.GetSequenceNumber()<<std::endl;
            GetDispatcher().HandleRequest(GetBroker(),m_message);
        }
        else if(m_message.GetStatus() != freedm::broker::CMessage::Created)
        {
            Logger.Debug<<"Rejected message "<<m_message.GetHash()<<":"
                          <<m_message.GetSequenceNumber()<<std::endl;
        }
listen:
        Logger.Debug<<"Listening for next message"<<std::endl;
        GetSocket().async_receive_from(boost::asio::buffer(m_buffer, CReliableConnection::MAX_PACKET_SIZE),
                m_endpoint, boost::bind(&CListener::HandleRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        GetConnectionManager().Stop(CListener::ConnectionPtr(this));	
    }
}
#pragma GCC diagnostic warning "-Wunused-label"

    } // namespace broker
} // namespace freedm
