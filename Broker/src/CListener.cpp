
////////////////////////////////////////////////////////////////////
/// @file      CConnection.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///            Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
///            Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description 
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be 
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
/// 
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes 
/// can be directed to Dr. Bruce McMillin, Department of 
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include "CDispatcher.hpp"
#include "CListener.hpp"
#include "CConnectionManager.hpp"
#include "CMessage.hpp"
#include "RequestParser.hpp"
#include "logger.hpp"
#include "config.hpp"

#include <vector>

#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

namespace freedm {
    namespace broker {
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
  CConnectionManager& p_manager, CDispatcher& p_dispatch, std::string uuid)
  : CReliableConnection(p_ioService,p_manager,p_dispatch,uuid)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CListener::Start
/// @description: Starts the recieve routine which causes this socket to behave
///   as a listener.
/// @pre The object is initialized.
/// @post The connection is asynchronously waiting for messages.
///////////////////////////////////////////////////////////////////////////////
void CListener::Start()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    GetSocket().async_receive_from(boost::asio::buffer(m_buffer, 8192), m_endpoint,
        boost::bind(&CListener::HandleRead, this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
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
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
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
void CListener::HandleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;       
    if (!e)
    {
        boost::tribool result_;
        boost::tie(result_, boost::tuples::ignore) = Parse(
            m_message, m_buffer.data(),
            m_buffer.data() + bytes_transferred);
        if (result_)
        {
            ptree x = static_cast<ptree>(m_message);
            std::string uuid = m_message.GetSourceUUID();
            remotehost hostname = m_message.GetSourceHostname();
            ///Make sure the hostname is registered:
            GetConnectionManager().PutHostname(uuid,hostname);                        
            ///Get the pointer to the connection:
            CConnection::ConnectionPtr conn;
            conn = GetConnectionManager().GetConnectionByUUID(uuid,
                GetSocket().get_io_service(), GetDispatcher());
            #ifdef CUSTOMNETWORK
            if((rand()%100) >= GetReliability())
            {
                goto listen;
            }
            #endif
            if(m_message.GetStatus() == freedm::broker::CMessage::Accepted)
            {
                conn->RecieveACK(m_message);
            }
            else if(conn->Recieve(m_message))
            {
                Logger::Notice<<"Accepted message "<<m_message.GetHash()<<":"
                              <<m_message.GetSequenceNumber()<<std::endl;
                GetDispatcher().HandleRequest(m_message);
            }

        }
        listen:
        GetSocket().async_receive_from(boost::asio::buffer(m_buffer, 8192), m_endpoint,
            boost::bind(&CListener::HandleRead, this,
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }
    else
    {
        GetConnectionManager().Stop(CListener::ConnectionPtr(this));	
    }
}

    } // namespace broker
} // namespace freedm
