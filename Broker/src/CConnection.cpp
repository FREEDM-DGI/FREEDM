////////////////////////////////////////////////////////////////////
/// @file      CConnection Header.cpp
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

CConnection::CConnection(boost::asio::io_service& p_ioService,
  CConnectionManager& p_manager, CDispatcher& p_dispatch)
  : m_socket( p_ioService),
    m_connManager( p_manager),
    m_dispatch( p_dispatch),
    m_timeout( p_ioService)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_uuid = "0";
    m_insequenceno = 0;
    m_outsequenceno = 0;
    m_timeouts = 0;
}

boost::asio::ip::tcp::socket& CConnection::GetSocket()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    return m_socket;
}

void CConnection::Start()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    Logger::Notice << "Connection Started, waiting for UUID" << std::endl;
    async_read(m_socket,boost::asio::buffer(m_buffer, UUIDLENGTH),
        boost::bind(&CConnection::HandleReadUUID, shared_from_this(),
            boost::asio::placeholders::error));
}

void CConnection::Stop()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_timeout.cancel();
    m_socket.close();
}

    
void CConnection::Send(CMessage p_mesg)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    //TODO: Call CDispatcher::HandleWrite() before actually queuing the message 
    //otherwise, outgoing event handlers won't be called such as the UUID.
    // At this point, it works without doing so. The main idea is that if you 
    //want a message to have an opportunity to be touched by other modules, the 
    //CDispatcher for be the first layer on incoming messages and the final 
    //layer on outgoing messages

    ptree x = static_cast< ptree>(p_mesg);
   
    //m_dispatch.HandleWrite(x); 
    m_outsequenceno++;
    
    // Container is thread-safe
    m_queue.Push( QueueItem(m_outsequenceno,p_mesg) );

    if(m_queue.size() <= WINDOWSIZE )
    {
        // Only try to write to the socket if the window isn't already full.
        m_socket.io_service().post(
            boost::bind(&CConnection::HandleSend, shared_from_this())
        );
    }
}

void CConnection::GiveUUID(std::string uuid)
{
    Logger::Notice << __PRETTY_FUNCTION__ << std::endl;
    boost::asio::async_write(m_socket, boost::asio::buffer(uuid),
        boost::bind(&CConnection::HandleWrite, shared_from_this(),
            boost::asio::placeholders::error));
}

void CConnection::HandleSend()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    boost::tribool result_;
    boost::array<char, 8192>::iterator it_;
    SlidingWindow<QueueItem>::iterator sit;
    CMessage msg_;
    QueueItem qfront;
    unsigned int sequenceno;

    it_ = m_outbuff.begin();
    sit = m_queue.begin();
    for(unsigned int i=0; sit != m_queue.end() && i < WINDOWSIZE; i++,sit++ )
    {
        msg_ = (*sit).second;
        sequenceno = (*sit).first;
        Logger::Debug << "Writing Msg #"<<sequenceno<<"To Channel"<<std::endl;
        boost::tie( result_, it_ ) = Synthesize( msg_, sequenceno, it_,
            m_outbuff.end() - it_ );
        // Create A Timer For Doing Resends:
        // Construct a timer using the socket.
        // Put the item into the list of expected ACKs
        m_timeout.cancel();
        m_timeout.expires_from_now(boost::posix_time::milliseconds(500));
        m_timeout.async_wait(boost::bind(&CConnection::Resend,this,
            boost::asio::placeholders::error));
    }

    // The buffer is safe since it can only be modified in this
    // thread AND the scheduler does FIFO, so it must be sent before
    // it can be modified again.
    boost::asio::async_write(m_socket,
        boost::asio::buffer(m_outbuff,
            (it_ - m_outbuff.begin()) * sizeof(char) ),
        boost::bind(&CConnection::HandleWrite, shared_from_this(),
            boost::asio::placeholders::error));
}

void CConnection::HandleSendACK(unsigned int sequenceno)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    CConnectionHeader x(sequenceno);
    std::string msg = x.ToString();
    Logger::Debug << "ACK #"<<sequenceno<<"To Channel"<<std::endl;
    boost::asio::async_write(m_socket,boost::asio::buffer(msg,msg.size()),
        boost::bind(&CConnection::HandleWrite, shared_from_this(),
            boost::asio::placeholders::error));
}

void CConnection::Resend(const boost::system::error_code& err)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if(!err)
    {
        Logger::Debug << "Firing Resend"<<std::endl;
        //There was no ack, we should pretty much do send, without
        if(!m_queue.IsEmpty())
        {
            // Only queue the send if there isn't already a send queued
            // This *should* group messages together that can be.
            m_socket.io_service().post(
                boost::bind(&CConnection::HandleSend, shared_from_this()));
        }
        m_timeouts++;
    }
    
}

std::string CConnection::GetUUID()
{
    return m_uuid;
}

void CConnection::SetUUID(std::string uuid)
{
    m_uuid = uuid;
}

void CConnection::HandleReadUUID(const boost::system::error_code& e)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    Logger::Notice << "Waiting For UUID " << std::endl;
    if(!e)
    {
        SetUUID(std::string(m_buffer.data(), UUIDLENGTH));
        Logger::Notice << "Got UUID " << GetUUID() << std::endl;
        m_connManager.RegisterConnection(GetUUID(),shared_from_this());
        async_read(m_socket,boost::asio::buffer(m_buffer, CConnectionHeader::HEADERSIZE),
            boost::bind(&CConnection::HandleReadHeader, shared_from_this(),
                boost::asio::placeholders::error));    
    }
    else
    {
        m_connManager.Stop(shared_from_this());	
    }
}

CConnectionHeader CConnection::ParseHeader()
{
    CConnectionHeader header(m_buffer,CConnectionHeader::HEADERSIZE);
    return header;
}

bool CConnection::CanParseHeader()
{
    return ParseHeader().IsWellFormed();
}

void CConnection::HandleReadHeader(const boost::system::error_code& e)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    Logger::Notice << "Waiting for Header" << std::endl;
    if(!e && CanParseHeader())
    {
        CConnectionHeader header = ParseHeader();
        if(header.IsAck())
        {
            if(!m_queue.IsEmpty())
            {
                Logger::Debug<<"Got ACK #"<<header.GetSequenceNumber()<<" "
                              <<m_queue.front().first<<" <= "<<header.GetSequenceNumber()
                              <<std::endl;
            }
            else
            {
                Logger::Debug<<"Got ACK #"<<header.GetSequenceNumber()<<std::endl;
            }
            m_timeout.cancel();
            // Pop out from the message queue until you find the right
            // Sequence no.
            while(!m_queue.IsEmpty() && m_queue.front().first <= header.GetSequenceNumber())
            {
                Logger::Debug << "Popping #" << m_queue.front().first << std::endl;
                m_queue.pop(); 
            }
            if(!m_queue.IsEmpty())
            {
                Logger::Debug << "Calling Send" << std::endl;
                m_socket.io_service().post(
                    boost::bind(&CConnection::HandleSend, shared_from_this()));
            }
            async_read(m_socket,boost::asio::buffer(m_buffer, CConnectionHeader::HEADERSIZE),
                boost::bind(&CConnection::HandleReadHeader, shared_from_this(),
                    boost::asio::placeholders::error)); 
        }
        else
        {
            unsigned int sequenceno = header.GetSequenceNumber();
            m_msg_len = header.GetMessageSize();
            Logger::Debug << "Got header of size "<<m_msg_len<<std::endl;
            Logger::Debug << "Got sequence number"<<sequenceno<<std::endl;
            if(sequenceno == m_insequenceno+1)
            {
                //The message was delievered in order. We can read it. 
                //Then read the message:
                Logger::Debug << "In Sequence Accepted "<<sequenceno<<std::endl;
                async_read(m_socket,boost::asio::buffer(m_buffer, m_msg_len),
                    boost::bind(&CConnection::HandleRead, shared_from_this(),
                        boost::asio::placeholders::error));
            }
            else
            {
                Logger::Debug << "Out of Sequence Dropped "<<sequenceno<<std::endl;
                async_read(m_socket,boost::asio::buffer(m_buffer, m_msg_len),
                    boost::bind(&CConnection::DropMessage, shared_from_this(),
                        boost::asio::placeholders::error));
            }
        }
    }
    else
    {
        m_connManager.Stop(shared_from_this());	
    }
}

void CConnection::DropMessage(const boost::system::error_code& e)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if(!e)
    {
        //Send an ACK for the last one we recieved:
        freedm::broker::CMessage m_;
        HandleSendACK(m_insequenceno);
        //Stuff has been placed in the buffer and has now been dropped
        async_read(m_socket,boost::asio::buffer(m_buffer, CConnectionHeader::HEADERSIZE),
            boost::bind(&CConnection::HandleReadHeader, shared_from_this(),
                boost::asio::placeholders::error)); 
    }
    else
    {
        m_connManager.Stop(shared_from_this());
    }
}

void CConnection::HandleRead(const boost::system::error_code& e)
{
    Logger::Notice << __PRETTY_FUNCTION__ << std::endl;       
    if (!e)
    {
        Logger::Notice << "Handled some message." << std::endl;
        boost::tribool result_;
        boost::tie(result_, boost::tuples::ignore) = Parse(
            m_message, m_buffer.data(),
            m_buffer.data() + m_msg_len);
        if (result_)
        {
            m_insequenceno++;
            freedm::broker::CMessage m_;
            HandleSendACK(m_insequenceno);
            m_dispatch.HandleRequest(static_cast< ptree >(m_message));
        }
        async_read(m_socket,boost::asio::buffer(m_buffer, CConnectionHeader::HEADERSIZE),
            boost::bind(&CConnection::HandleReadHeader, shared_from_this(),
                boost::asio::placeholders::error)); 
    }
    else
    {
        m_connManager.Stop(shared_from_this());	
    }
}

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
