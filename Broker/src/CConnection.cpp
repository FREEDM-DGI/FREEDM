
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
#include "CConnection.hpp"
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
/// @fn CConnection::CConnection
/// @description Constructor for the CConnection object. Since the change to
///   udp, this object can act either as a listener or sender (but not both)
///   to have the object behave as a listener, Start() should be called on it.
/// @pre An initialized socket is ready to be converted to a connection.
/// @post A new CConnection object is initialized.
/// @param p_ioService The socket to use for the connection.
/// @param p_manager The related connection manager that tracks this object.
/// @param p_dispatch The dispatcher responsible for applying read/write 
///   handlers to messages.
/// @param uuid The uuid this node connects to, or what listener.
///////////////////////////////////////////////////////////////////////////////
CConnection::CConnection(boost::asio::io_service& p_ioService,
  CConnectionManager& p_manager, CDispatcher& p_dispatch, std::string uuid)
  : CReliableConnection(p_ioService,p_manager,p_dispatch,uuid),
    m_timeout( p_ioService)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_outsequenceno = 0;
    m_timeouts = 0;
    m_synched = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Start
/// @description Starts the recieve routine which causes this socket to behave
///   as a listener.
/// @pre The object is initialized.
/// @post The connection is asynchronously waiting for messages.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Start()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Stop
/// @description Stops the socket and cancels the timeout timer. Does not
///   need to be called on a listening connection (ie one that has had
///   Start() called on it.
/// @pre Any initialized CConnection object.
/// @post The underlying socket is closed and the message timeout timer is
///        cancelled.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Stop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_timeout.cancel();
    GetSocket().close();
}
 
///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Send
/// @description Given a message and wether or not it should be sequenced,
///   write that message to the channel.
/// @pre The CConnection object is initialized.
/// @post If the window is in not full, the message will have been written to
///   to the channel. Before being sent the message has been signed with the
///   UUID, source hostname and sequence number (if it is being sequenced).
///   If the message is being sequenced  and the window is not already full,
///   the timeout timer is cancelled and reset.
/// @param p_mesg A CMessage to write to the channel.
/// @param sequence if true, the message will be sequenced and reliably
///   delievered in order. Otherwise it is immediately fired and forgotten.
///   this is mostly meant for use with ACKs. True by default.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Send(CMessage p_mesg, bool sequence)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    #ifdef DATAGRAM
    sequence = false;
    #endif

    //Make a call to the dispatcher to sign the messages
    //With a bunch of shiny stuff.
    ptree x = static_cast<ptree>(p_mesg);
    unsigned int msgseq;

    //m_dispatch.HandleWrite(x);  

    CMessage outmsg(x);

    // Sign the message with the hostname, uuid, and squencenumber
    if(sequence == true)
    {
        if(m_synched == false)
        {
            m_synched = true;
            SendSYN();
        }
        msgseq = m_outsequenceno;
        outmsg.SetSequenceNumber(msgseq);
        m_outsequenceno = (m_outsequenceno+1) % GetSequenceModulo();
    }
    outmsg.SetSourceUUID(GetConnectionManager().GetUUID()); 
    outmsg.SetSourceHostname(GetConnectionManager().GetHostname());

    if(sequence == true)
    {
        // If it isn't squenced then don't put it in the queue.
        m_queue.Push( QueueItem(msgseq,outmsg) );
    }
    // Before, we would put it into a queue to be sent later, now we are going
    // to immediately write it to channel.

    if(m_queue.size() <= GetWindowSize() || sequence == false)
    {
        // Only try to write to the socket if the window isn't already full.
        // Or it is an unsequenced message
        
        HandleSend(outmsg);
        if(sequence == true)
        {
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(500));
            m_timeout.async_wait(boost::bind(&CConnection::Resend,this,
                boost::asio::placeholders::error));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::HandleSend
/// @description This function synthesizes the input CMessage and writes it to
///   the channel.
/// @pre The CConnection is initialized.
/// @post A CMessage has been written to the socket for this connection.
///////////////////////////////////////////////////////////////////////////////
void CConnection::HandleSend(CMessage msg)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::tribool result_;
    boost::array<char, 8192>::iterator it_;

    it_ = m_buffer.begin();
    boost::tie( result_, it_ ) = Synthesize( msg, it_, m_buffer.end() - it_ );

    #ifdef CUSTOMNETWORK
    if((rand()%100) >= GetReliability()) 
    {
        Logger::Info<<"Outgoing Packet Dropped ("<<GetReliability()
                      <<") -> "<<GetUUID()<<std::endl;
        return;
    }
    #endif

    GetSocket().async_send(boost::asio::buffer(m_buffer,
            (it_ - m_buffer.begin()) * sizeof(char) ), 
        boost::bind(&CConnection::HandleWrite, this,
        boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Resend
/// @description The callback for the timeout timer. If the timeout timer
///   expires and the message queue is not empty, this function marks that
///   there was a timeout and calls the HandleResend method to refire the
///   window. Otherwise, this function simply exits.
/// @param err The error code for the expiring timer.
/// @pre The timeout timer has just expired or been cancelled.
/// @post If the timer was not cancelled and there are messages in the queue
///   the queue is resent.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Resend(const boost::system::error_code& err)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if(!err)
    {
        Logger::Debug << "Firing Resend"<<std::endl;
        if(!m_queue.IsEmpty())
        {
            m_timeouts++;
            GetSocket().get_io_service().post(
            boost::bind(&CConnection::HandleResend, this));
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(100));
            m_timeout.async_wait(boost::bind(&CConnection::Resend,this,
                boost::asio::placeholders::error));
        }
    }
    
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::HandleResend
/// @description After being called, this method sends up WINDOWSIZE messages
///   to retry delievery.
/// @pre Initialized CConnection.
/// @post Upto WINDOWSIZE messages are rewritten to the channel.
///////////////////////////////////////////////////////////////////////////////
void CConnection::HandleResend()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    SlidingWindow<QueueItem>::iterator sit;
    sit = m_queue.begin();   
 
    for(unsigned int i=0; sit != m_queue.end() && i < GetWindowSize(); i++,sit++ )
    {
        GetSocket().get_io_service().post(
        boost::bind(&CConnection::HandleSend, this,(*sit).second));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::SendSYN
/// @description Synchronosizes the reciever to expect the next message to have
///   the sequence number 1
/// @pre Initialized connection.
/// @post Upon reciept, the reciever will expect 1 for the sequence number
///////////////////////////////////////////////////////////////////////////////
void CConnection::SendSYN()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    freedm::broker::CMessage m_;
    m_.SetStatus(freedm::broker::CMessage::Created);
    Logger::Info<<"Sending SYN"<<std::endl;
    Send(m_);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::RecieveACK
/// @description Handler for recieving acknowledgments from a sender.
/// @pre Initialized connection.
/// @post The message with sequence number has been acknowledged and all
///   messages sent before that message have been considered acknowledged as
///   well.
/// @param sequenceno The message to consider as acknowledged.
///////////////////////////////////////////////////////////////////////////////
void CConnection::RecieveACK(unsigned int sequenceno)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    while(!m_queue.IsEmpty())
    {
        unsigned int bounda = m_queue.front().first;
        unsigned int boundb = (m_queue.front().first+(GetWindowSize()))%GetSequenceModulo();
        Logger::Debug<<"ACK, bounda:"<<bounda<<" boundb:"<<boundb<<"input: "<<sequenceno<<std::endl;
        if(bounda <= sequenceno || (sequenceno < boundb && boundb < bounda))
        {
            Logger::Debug<<"ACK handled for "<<m_queue.front().first<<std::endl;
            m_queue.pop();
            m_timeouts = 0;
        }
        else
        {
            break;
        }
    }
    if(!m_queue.IsEmpty())
    {
        GetSocket().get_io_service().post(
            boost::bind(&CConnection::HandleResend, this));
    }
}
///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::HandleWrite
/// @description Write callback. Closes the connection on error, does nothing
///   otherwise.
/// @param e The error that occured if any.
/// @pre A message has been written to the channel.
/// @post If there was an error the socket has been closed.
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
        GetConnectionManager().Stop(CConnection::ConnectionPtr(this));
    }
}

    } // namespace broker
} // namespace freedm
