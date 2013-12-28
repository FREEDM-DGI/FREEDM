////////////////////////////////////////////////////////////////////////////////
/// @file         CSRConnection.cpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CSRConnection class
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
#include "CLogger.hpp"
#include "CMessage.hpp"
#include "CSRConnection.hpp"
#include "IProtocol.hpp"
#include "CTimings.hpp"

#include <iomanip>
#include <set>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::CSRConnection
/// @description Constructor for the CSRConnection class.
/// @pre The object is uninitialized.
/// @post The object is initialized: m_killwindow is empty, the connection is
///       marked as unsynced, It won't be sending kill statuses. Its first
///       message will be numbered as 0 for outgoing and the timer is not set.
/// @param conn The underlying connection object this protocol writes to
///////////////////////////////////////////////////////////////////////////////
CSRConnection::CSRConnection(CConnection *  conn)
    : IProtocol(conn),
      m_timeout(conn->GetSocket().get_io_service())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    //Sequence Numbers
    m_outseq = 0;
    m_inseq = 0;
    //Inbound Message Sequencing
    m_insync = false;
    //m_insynctime
    m_inresyncs = 0;
    //Outbound message sequencing
    m_outsync = false;
    // Message killing (SEND)
    m_sendkills = false;
    m_sendkill = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::CSRConnection
/// @description Send function for the CSRConnection. Sending using this
///   protocol involves an alternating bit scheme. Messages can expire and
///   delivery won't be attempted after the deadline is passed. Killed messages
///   are noted in the next outgoing message. The receiver tracks the killed
///   messages and uses them to help maintain ordering.
/// @pre The protocol is intialized.
/// @post At least one message is in the channel and actively being resent.
///     The send window is greater than or equal to one. The timer for the
///     resend is freshly set or is currently running for a resend.
///     If a message is written to the channel, the m_killable flag is set.
/// @param msg The message to write to the channel.
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::Send(CMessage msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    unsigned int msgseq;

    if(m_outsync == false)
    {
        SendSYN();
    }


    msgseq = m_outseq;
    msg.SetSequenceNumber(msgseq);
    m_outseq = (m_outseq+1) % SEQUENCE_MODULO;

    msg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
    msg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHost());
    msg.SetProtocol(GetIdentifier());
    msg.SetSendTimestampNow();
    if(!msg.HasExpireTime())
    {
        Logger.Debug<<"Set Expire time"<<std::endl;
        msg.SetExpireTimeFromNow(boost::posix_time::milliseconds(CTimings::CSRC_DEFAULT_TIMEOUT));
    }

    if(m_window.size() == 0)
    {
        Write(msg);
        boost::system::error_code x;
        Resend(x);
    }
    m_window.push_back(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::CSRConnection
/// @description Handles refiring ACKs and Sent Messages.
/// @pre The connection has received or sent at least one message.
/// @post One of the following conditions or combination of states is
///       upheld:
///       1) An ack for a message that has not yet expired has been resent and
///          a timer to call resend has been set.
///       2) Message(s) has/have expired and are removed from the queue. The
///          flag to send kills is set.
///       3) The window is empty and no message is set to the channel, the
///          timer is not re-set.
///       4) A message expired and then next message will cause the sequence
///          numbers to wrap, (or they have wrapped since the last time a message
///          was successfully sent) so a sync is inserted at the front of the queue
///          to skip that case on the receiver side. The sendkill flag is cleared
///          and the sendkill value is cleared.
///       5) If there is still a message to resend, the timer is reset.
/// @param err The timer error code. If the err is 0 then the timer expired
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::Resend(const boost::system::error_code& err)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    if(!err && !GetStopped())
    {
        Logger.Trace<<__PRETTY_FUNCTION__<<" Checking ACK"<<std::endl;
        // Check if the front of the queue is an ACK
        /*
        m_ackmutex.lock();
        if(m_currentack.GetStatus() == freedm::broker::CMessage::Accepted)
        {
            //if(!m_currentack.IsExpired())
            //{
                Write(m_currentack);
                m_timeout.cancel();
                m_timeout.expires_from_now(boost::posix_time::milliseconds(REFIRE_TIME));
                m_timeout.async_wait(boost::bind(&CSRConnection::Resend,this,
                    boost::asio::placeholders::error));
            //}
        }
        m_ackmutex.unlock();
        */
        Logger.Trace<<__PRETTY_FUNCTION__<<" Sent ACK"<<std::endl;
        while(m_window.size() > 0 && m_window.front().IsExpired())
        {
            Logger.Trace<<__PRETTY_FUNCTION__<<" Flusing"<<std::endl;
            //First message in the window should be the only one
            //ever to have been written.
            m_sendkills = true;
            Logger.Debug<<"Message Expired: "<<m_window.front().GetHash()
                          <<":"<<m_window.front().GetSequenceNumber()<<std::endl;
            m_window.pop_front();
        }
        Logger.Trace<<__PRETTY_FUNCTION__<<" Flushed Expired"<<std::endl;
        if(m_window.size() > 0)
        {
            if(m_sendkills &&  m_sendkill > m_window.front().GetSequenceNumber())
            {
                // If we have expired a message and caused the seqnos
                // to wrap, we resync the connection. This shouldn't
                // happen very often.
                // If we wrap, don't send kills
                m_sendkills = false;
                m_sendkill = 0;
                SendSYN();
            }
            if(m_sendkills)
            {
                // kill will be set to the last message accepted by receiver
                // (and whose ack has been received)
                Logger.Trace<<__PRETTY_FUNCTION__<<" Adding Properties"<<std::endl;
                ptree x;
                x.put("src.kill",m_sendkill);
                m_window.front().SetProtocolProperties(x);
            }
            Logger.Trace<<__PRETTY_FUNCTION__<<" Writing"<<std::endl;
            Write(m_window.front());
            // Head of window can be killed.
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(CTimings::CSRC_RESEND_TIME));
            m_timeout.async_wait(boost::bind(&CSRConnection::Resend,shared_from_this(),
                boost::asio::placeholders::error));
        }
    }
    Logger.Trace<<__PRETTY_FUNCTION__<<" Resend Finished"<<std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::ReceiveACK
/// @description Marks a message as acknowledged by the receiver and moves to
///     transmit the next message.
/// @pre A message has been sent.
/// @post If the ACK corresponds to the head of window by a match of sequence
///       number and the message hash, the the message is popped and the
///       killable flag is set to false, since the head of the window has never
///       been sent.
///       If the there is still an message in the window to send, the
///       resend function is called.
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::ReceiveACK(const CMessage &msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int seq = msg.GetSequenceNumber();
    ptree pp = msg.GetProtocolProperties();
    size_t hash = pp.get<size_t>("src.hash");
    if(m_window.size() > 0)
    {
        // Assuming hash collisions are small, we will check the hash
        // of the front message. On hit, we can accept the acknowledge.
        unsigned int fseq = m_window.front().GetSequenceNumber();
        Logger.Debug<<"Received ACK "<<seq<<" expecting ACK "<<fseq<<std::endl;
        if(fseq == seq && m_window.front().GetHash() == hash)
        {
            m_sendkill = fseq;
            m_window.pop_front();
            m_sendkills = false;
        }
    }
    if(m_window.size() > 0)
    {
        boost::system::error_code x;
        Resend(x);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::Receive
/// @description Accepts a message into the protocol, if that message should
///   be accepted. If this function returns true, the message is passed to
///   the dispatcher. Since this message accepts SYNs there might be times
///   when processing and state changes but the message is marked as "rejected"
///   this is normal.
/// @pre Accept logic can be complicated, there are several scenarios that
///      should be addressed.
///      1) A bad request has been received
///      2) A SYN message is received for the first time
///      3) A SYN message is received as a duplicate.
///      4) A Message has been received before the connection has been synced.
///      5) A Message has been received with the expected sequenceno with or
///         without a kill flag.
///      6) A message has been received with a kill flag. The kill is greater
///         than the expected sequence number
///      7) A message has been received with a kill flag. The kill is less than
///         the expected sequence number. However, the message's number is less
///         than the expected sequence number
///      8) A message has been received with a kill flag. The kill is less than
///         the expected sequence number and the message's sequence number is
///         greater than the expected sequence number.
/// @post Cases are handled as follows:
///      1) The connection is resynced.
///      2) The message is ACKed, the send time of the sync is noted, and the
///         connection is synced.
///      3) The SYN is ignored.
///      4) A bad request message is generated and sent to the source.
///      5) The message is accepted.
///      6) The message is rejected. Kills should only ever be less than the
///         expected sequence number unless the message is arrived out of order
///      7) The message is simply old but still arriving at the socket, and can
///         be rejected.
///      8) The message should be accepted because one or more message expired
///         in the gap of sequence numbers.
/// @return True if the message is accepted, false otherwise.
///////////////////////////////////////////////////////////////////////////////
bool CSRConnection::Receive(const CMessage &msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int kill = 0;
    bool usekill = false; //If true, we should accept any inseq
    if(msg.GetStatus() == freedm::broker::CMessage::BadRequest)
    {
        //See if we are already trying to sync:
        if(m_window.front().GetStatus() != freedm::broker::CMessage::Created)
        {
            if(m_outsynctime != msg.GetSendTimestamp())
            {
                Logger.Debug<<"Syncronizing Connection (BAD REQUEST)"<<std::endl;
                m_outsynctime = msg.GetSendTimestamp();
                SendSYN();
            }
            else
            {
                Logger.Debug<<"Already synced for this time"<<std::endl;
            }
        }
        return false;
    }
    if(msg.GetStatus() == freedm::broker::CMessage::Created)
    {
        //Check to see if we've already seen this SYN:
        if(msg.GetSendTimestamp() == m_insynctime)
        {
            return false;
            Logger.Debug<<"Duplicate Sync"<<std::endl;
        }
        Logger.Debug<<"Got Sync"<<std::endl;
        m_inseq = (msg.GetSequenceNumber()+1)%SEQUENCE_MODULO;
        m_insynctime = msg.GetSendTimestamp();
        m_inresyncs++;
        m_insync = true;
        SendACK(msg);
        return false;
    }
    if(m_insync == false)
    {
        Logger.Debug<<"Connection Needs Resync"<<std::endl;
        //If the connection hasn't been synchronized, we want to
        //tell them it is a bad request so they know they need to sync.
        freedm::broker::CMessage outmsg;
        // Presumably, if we are here, the connection is registered
        outmsg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
        outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHost());
        outmsg.SetStatus(freedm::broker::CMessage::BadRequest);
        outmsg.SetSequenceNumber(m_inresyncs%SEQUENCE_MODULO);
        outmsg.SetSendTimestamp(msg.GetSendTimestamp());
        outmsg.SetProtocol(GetIdentifier());
        Write(outmsg);
        return false;
    }
    // See if the message contains kill data. If it does, read it and mark
    // we should use it.
    try
    {
        ptree pp = msg.GetProtocolProperties();
        kill = pp.get<unsigned int>("src.kill");
        usekill = true;
    }
    catch(std::exception &e)
    {
        kill = msg.GetSequenceNumber();
        usekill = false;
    }
    //Consider the window you expect to see
    // If the killed message is the one immediately preceeding this
    // message in terms of sequence number we should accept it
    Logger.Debug<<"Recv: "<<msg.GetSequenceNumber()<<" Expected "<<m_inseq<<" Using kill: "<<usekill<<" with "<<kill<<std::endl;
    if(msg.GetSequenceNumber() == m_inseq)
    {
        //m_insync = true;
        m_inseq = (m_inseq+1)%SEQUENCE_MODULO;
        return true;
    }
    else if(usekill == true && kill< m_inseq
            && msg.GetSequenceNumber() > m_inseq)
    {
        //m_inseq will be right for the next expected message.
        m_inseq = (msg.GetSequenceNumber()+1)%SEQUENCE_MODULO;
        return true;
    }
    else if(usekill == true)
    {
        Logger.Debug<<"KILL: "<<kill<<" INSEQ "<<m_inseq<<" SEQ: "
                      <<msg.GetSequenceNumber()<<std::endl;
    }
    // Justin case.
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::SendACK
/// @description Composes an ack and writes it to the channel. ACKS are saved
///     to the protocol's state and are written again during resends to try and
///     maximize througput.
/// @param msg The message to ACK.
/// @pre A message has been accepted.
/// @post The m_currentack member is set to the ack and the message will
///     be resent during resend until it expires.
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::SendACK(const CMessage &msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int seq = msg.GetSequenceNumber();
    freedm::broker::CMessage outmsg;
    ptree pp;
    pp.put("src.hash",msg.GetHash());
    // Presumably, if we are here, the connection is registered
    outmsg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
    outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHost());
    outmsg.SetStatus(freedm::broker::CMessage::Accepted);
    outmsg.SetSequenceNumber(seq);
    outmsg.SetSendTimestampNow();
    outmsg.SetProtocol(GetIdentifier());
    outmsg.SetProtocolProperties(pp);
    Logger.Debug<<"Generating ACK. Source exp time "<<msg.GetExpireTime()<<std::endl;
    outmsg.SetExpireTime(msg.GetExpireTime());
    Write(outmsg);
    m_ackmutex.lock();
    m_currentack = outmsg;
    m_ackmutex.unlock();
    /// Hook into resend until the message expires.
    m_timeout.cancel();
    m_timeout.expires_from_now(boost::posix_time::milliseconds(CTimings::CSRC_RESEND_TIME));
    m_timeout.async_wait(boost::bind(&CSRConnection::Resend,shared_from_this(),
        boost::asio::placeholders::error));
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::SendSYN
/// @description Composes an SYN and writes it to the channel.
/// @pre A message has been accepted.
/// @post A syn has been written to the channel
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::SendSYN()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int seq = m_outseq;
    freedm::broker::CMessage outmsg;
    if(m_window.size() == 0)
    {
        m_outseq = (m_outseq+1)%SEQUENCE_MODULO;
    }
    else
    {
        //Don't bother if front of queue is already a SYN
        if(m_window.front().GetStatus() == CMessage::Created)
        {
            return;
        }
        //Set it as the seq before the front of queue
        seq = m_window.front().GetSequenceNumber();
        if(seq == 0)
        {
            seq = SEQUENCE_MODULO-1;
        }
        else
        {
            seq--;
        }
    }
    // Presumably, if we are here, the connection is registered
    outmsg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
    outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHost());
    outmsg.SetStatus(freedm::broker::CMessage::Created);
    outmsg.SetSequenceNumber(seq);
    outmsg.SetSendTimestampNow();
    outmsg.SetProtocol(GetIdentifier());
    outmsg.SetNeverExpires();
    Write(outmsg);
    m_window.push_front(outmsg);
    m_outsync = true;
    /// Hook into resend until the message expires.
    boost::system::error_code x;
    Resend(x);
}

    }
}
