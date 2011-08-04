
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
#include "CConnectionHeader.hpp"
#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CMessage.hpp"
#include "RequestParser.hpp"
#include "logger.hpp"


#include <vector>

#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

namespace freedm {
    namespace broker {
///////////////////////////////////////////////////////////////////////////////
/// CConnection::CConnection
/// @description: Constructor for the CConnection object. Since the change to
///   udp, this object can act either as a listener or sender (but not both)
///   to have the object behave as a listener, Start() should be called on it.
/// @pre: An initialized socket is ready to be converted to a connection.
/// @post: A new CConnection object is initialized.
/// @param p_ioService: The socket to use for the connection.
/// @param p_manager: The related connection manager that tracks this object.
/// @param p_dispatch: The dispatcher responsible for applying read/write 
///   handlers to messages.
/// @param uuid: The uuid this node connects to, or what listener.
///////////////////////////////////////////////////////////////////////////////
CConnection::CConnection(boost::asio::io_service& p_ioService,
  CConnectionManager& p_manager, CDispatcher& p_dispatch, std::string uuid)
  : m_socket( p_ioService),
    m_connManager( p_manager),
    m_dispatch( p_dispatch),
    m_timeout( p_ioService)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_uuid = uuid;
    m_outsequenceno = 0;
    m_timeouts = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::GetSocket
/// @description Returns the socket used by this node.
/// @pre None
/// @post None
/// @return A reference to the socket used by this connection.
///////////////////////////////////////////////////////////////////////////////
boost::asio::ip::udp::socket& CConnection::GetSocket()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    return m_socket;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::Start
/// @description: Starts the recieve routine which causes this socket to behave
///   as a listener.
/// @pre: The object is initialized.
/// @post: The connection is asynchronously waiting for messages.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Start()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    Logger::Notice << "Connection Started" << std::endl;
    m_socket.async_receive_from(boost::asio::buffer(m_buffer, 8192), m_endpoint,
        boost::bind(&CConnection::HandleRead, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::Stop
/// @description: Stops the socket and cancels the timeout timer. Does not
///   need to be called on a listening connection (ie one that has had
///   Start() called on it.
/// @pre: Any initialized CConnection object.
/// @post: The underlying socket is closed and the message timeout timer is
///        cancelled.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Stop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_timeout.cancel();
    m_socket.close();
}
 
///////////////////////////////////////////////////////////////////////////////
/// CConnection::Send
/// @description: Given a message and wether or not it should be sequenced,
///   write that message to the channel.
/// @pre: The CConnection object is initialized.
/// @post: If the window is in not full, the message will have been written to
///   to the channel. Before being sent the message has been signed with the
///   UUID, source hostname and sequence number (if it is being sequenced).
///   If the message is being sequenced  and the window is not already full,
///   the timeout timer is cancelled and reset.
/// @param p_mesg: A CMessage to write to the channel.
/// @param sequence: if true, the message will be sequenced and reliably
///   delievered in order. Otherwise it is immediately fired and forgotten.
///   this is mostly meant for use with ACKs. True by default.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Send(CMessage p_mesg, bool sequence)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    //Make a call to the dispatcher to sign the messages
    //With a bunch of shiny stuff.
    ptree x = static_cast<ptree>(p_mesg);

    //m_dispatch.HandleWrite(x);  

    CMessage outmsg(x);

    // Sign the message with the hostname, uuid, and squencenumber
    if(sequence == true)
    {
        m_outsequenceno++;
        outmsg.SetSequenceNumber(m_outsequenceno);
    }
    outmsg.SetSourceUUID(m_connManager.GetUUID()); 
    outmsg.SetSourceHostname(m_connManager.GetHostname());

    if(sequence == true)
    {
        // If it isn't squenced then don't put it in the queue.
        m_queue.Push( QueueItem(m_outsequenceno,outmsg) );
    }
    // Before, we would put it into a queue to be sent later, now we are going
    // to immediately write it to channel.

    if(m_queue.size() <= WINDOWSIZE || sequence == false)
    {
        // Only try to write to the socket if the window isn't already full.
        // Or it is an unsequenced message
        HandleSend(outmsg);
        if(sequence == true)
        {
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(1000));
            m_timeout.async_wait(boost::bind(&CConnection::Resend,this,
                boost::asio::placeholders::error));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::HandleSend
/// @description: This function synthesizes the input CMessage and writes it to
///   the channel.
/// @pre: The CConnection is initialized.
/// @post: A CMessage has been written to the socket for this connection.
///////////////////////////////////////////////////////////////////////////////
void CConnection::HandleSend(CMessage msg)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::tribool result_;
    boost::array<char, 8192>::iterator it_;

    it_ = m_buffer.begin();
    boost::tie( result_, it_ ) = Synthesize( msg, it_, m_buffer.end() - it_ );

    m_socket.async_send(boost::asio::buffer(m_buffer,
            (it_ - m_buffer.begin()) * sizeof(char) ), 
        boost::bind(&CConnection::HandleWrite, shared_from_this(),
        boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::Resend
/// @description: The callback for the timeout timer. If the timeout timer
///   expires and the message queue is not empty, this function marks that
///   there was a timeout and calls the HandleResend method to refire the
///   window. Otherwise, this function simply exits.
/// @param err: The error code for the expiring timer.
/// @pre: The timeout timer has just expired or been cancelled.
/// @post: If the timer was not cancelled and there are messages in the queue
///   the queue is resent.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Resend(const boost::system::error_code& err)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if(!err)
    {
        Logger::Debug << "Firing Resend"<<std::endl;
        //There was no ack, we should pretty much do send, without
        if(!m_queue.IsEmpty())
        {
            m_timeouts++;
            m_socket.get_io_service().post(
            boost::bind(&CConnection::HandleResend, shared_from_this()));
        }
    }
    
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::HandleResend
/// @description: After being called, this method sends up WINDOWSIZE messages
///   to retry delievery.
/// @pre: Initialized CConnection.
/// @post: Upto WINDOWSIZE messages are rewritten to the channel.
///////////////////////////////////////////////////////////////////////////////
void CConnection::HandleResend()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    SlidingWindow<QueueItem>::iterator sit;
    sit = m_queue.begin();   
 
    for(unsigned int i=0; sit != m_queue.end() && i < WINDOWSIZE; i++,sit++ )
    {
        m_socket.get_io_service().post(
        boost::bind(&CConnection::HandleSend, shared_from_this(),(*sit).second));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::SendACK
/// @description: Sends an acknowledgement of message reciept back to sender.
/// @param uuid: The uuid to send the message to.
/// @param hostname: The hostname to send the message to.
/// @param sequenceno: The message number being acked.
/// @pre: Initialized connection.
/// @post: If the sender is not already in the hostname table it will be added.
///   then a connection is established to that node and an acknowledgment sent.
//////////////////////////////////////////////////////////////////////////////
void CConnection::SendACK(std::string uuid, std:: string hostname, unsigned int sequenceno)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    freedm::broker::CMessage m_;
    // Make sure the connection is registered:
    m_connManager.PutHostname(uuid,hostname);
    m_.SetStatus(freedm::broker::CMessage::Accepted);
    m_.SetSequenceNumber(sequenceno);
    Logger::Notice<<"Send ACK #"<<sequenceno<<std::endl;
    m_connManager.GetConnectionByUUID(uuid, m_socket.get_io_service(), m_dispatch)->Send(m_,false);
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::RecieveACK
/// @description: Handler for recieving acknowledgments from a sender.
/// @pre: Initialized connection.
/// @post: The message with sequence number has been acknowledged and all
///   messages sent before that message have been considered acknowledged as
///   well.
/// @param sequenceno: The message to consider as acknowledged.
///////////////////////////////////////////////////////////////////////////////
void CConnection::RecieveACK(unsigned int sequenceno)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    while(!m_queue.IsEmpty() && m_queue.front().first <= sequenceno)
    {
        Logger::Notice<<"ACK handled for "<<m_queue.front().first<<std::endl;
        m_queue.pop();
    }
    if(!m_queue.IsEmpty())
    {
        m_socket.get_io_service().post(
            boost::bind(&CConnection::HandleResend, shared_from_this()));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::HandleRead
/// @description: The callback which accepts messages from the remote sender.
/// @param e: The errorcode if any associated.
/// @param bytes_transferred: The size of the datagram being read.
/// @pre: The connection has had start called and some message has been placed
///   in the buffer by the recieve call.
/// @post: The message has been delivered. This means that write connections
///   have been notified of ACK and standard messages have been redirected to
///   their appropriate places by the dispatcher. The incoming sequence number
///   for the source UUID has been incremented appropriately.
///////////////////////////////////////////////////////////////////////////////
void CConnection::HandleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    Logger::Notice << __PRETTY_FUNCTION__ << std::endl;       
    if (!e)
    {
        Logger::Notice << "Handled some message." << std::endl;
        boost::tribool result_;
        boost::tie(result_, boost::tuples::ignore) = Parse(
            m_message, m_buffer.data(),
            m_buffer.data() + bytes_transferred);
        if (result_)
        {
            // Unfortunately, this can't be done with the read handler
            // So it has to be a bit messier. m_message is the incoming
            // CMessage. We scan this for the stamp that we place on a
            // Message before we send it with the sequence number:
            ptree x = static_cast<ptree>(m_message); 
            unsigned int sequenceno = m_message.GetSequenceNumber();
            std::string uuid = m_message.GetSourceUUID();
            std::string hostname = m_message.GetSourceHostname();
            if(m_message.GetStatus() == freedm::broker::CMessage::Accepted)
            {
                Logger::Notice << "Got ACK #" << sequenceno << std::endl;
                m_connManager.PutHostname(uuid,hostname);
                m_connManager.GetConnectionByUUID(uuid, m_socket.get_io_service(), m_dispatch)->RecieveACK(sequenceno);
            }
            else
            {
                if(m_insequenceno.find(uuid) == m_insequenceno.end())
                {
                    m_insequenceno[uuid] = 0;
                }
                Logger::Notice << "Got Message #" << sequenceno << " expected " << m_insequenceno[uuid]+1 << std::endl;
                if(sequenceno == m_insequenceno[uuid]+1)
                {
                    // The sequence number is what we expect.
                    m_insequenceno[uuid]++;
                    // Call on the connection manager to send the ACK to the sender.
                    SendACK(uuid,hostname,m_insequenceno[uuid]);
                    // Handle the request
                    // XXX: Dr. McMillin and scj discussed how to make this secure,
                    // specifically that messages from unknown sources should not
                    // be accepted unless some trusted agent (like the group manager)
                    // has accepted them. What I think I want to propose is that
                    // write now, this is written to accept all messages, but
                    // later we'll end in a "trust table" that epidemically
                    // distributes trust in the system. Messages are rejected until
                    // Trust is established or something. 
                    m_dispatch.HandleRequest(x);
                }
                else
                {
                    SendACK(uuid,hostname,m_insequenceno[uuid]);
                }
            }
        }
        m_socket.async_receive_from(boost::asio::buffer(m_buffer, 8192), m_endpoint,
            boost::bind(&CConnection::HandleRead, shared_from_this(),
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }
    else
    {
        m_connManager.Stop(shared_from_this());	
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::HandleWrite
/// @description: Write callback. Closes the connection on error, does nothing
///   otherwise.
/// @param e: The error that occured if any.
/// @pre: A message has been written to the channel.
/// @post: If there was an error the socket has been closed.
///////////////////////////////////////////////////////////////////////////////
void CConnection::HandleWrite(const boost::system::error_code& e)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    if (!e)
    {
        //All good
    }
    if (e == boost::asio::error::operation_aborted)
    {
        m_connManager.Stop(shared_from_this());
    }
}

    } // namespace broker
} // namespace freedm
