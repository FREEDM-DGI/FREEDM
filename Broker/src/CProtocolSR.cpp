////////////////////////////////////////////////////////////////////////////////
/// @file         CProtocolSR.cpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CProtocolSR class
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

#include "CLogger.hpp"
#include "CProtocolSR.hpp"
#include "CTimings.hpp"
#include "CBroker.hpp"

#include "Messages.hpp"
#include "messages/ProtocolMessage.pb.h"

#include <cassert>
#include <iomanip>
#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <google/protobuf/message.h>

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::CProtocolSR
/// @description Constructor for the CProtocolSR class.
/// @pre The object is uninitialized.
/// @post The object is initialized: m_killwindow is empty, the connection is
///       marked as unsynced, It won't be sending kill statuses. Its first
///       message will be numbered as 0 for outgoing and the timer is not set.
/// @param uuid The peer this connection is made to.
/// @param endpoint The endpoint that will be the destination for sent messages
///////////////////////////////////////////////////////////////////////////////
CProtocolSR::CProtocolSR(std::string uuid, boost::asio::ip::udp::endpoint endpoint)
    : IProtocol(uuid, endpoint),
      m_timeout(CBroker::Instance().GetIOService()),
      m_timer_active(false)
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
    m_dropped = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::CProtocolSR
/// @description Send function for the CProtocolSR. Sending using this
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
void CProtocolSR::Send(const ModuleMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_outsync == false)
    {
        SendSYN();
    }
    
    ProtocolMessage pm;
    pm.mutable_module_message()->CopyFrom(msg);

    unsigned int msgseq = m_outseq;
    pm.set_sequence_num(msgseq);
    m_outseq = (m_outseq+1) % SEQUENCE_MODULO;
    pm.set_hash(ComputeMessageHash(pm.module_message()));
    pm.set_status(ProtocolMessage::MESSAGE);

    SetExpirationTimeFromNow(pm, boost::posix_time::millisec(CTimings::Get("CSRC_DEFAULT_TIMEOUT")));
    Logger.Debug<<"Set Expire time: "<< pm.expire_time() << std::endl;

	m_window.push_back(pm);
    boost::system::error_code x;
    Resend(x);
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::CProtocolSR
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
void CProtocolSR::Resend(const boost::system::error_code& err)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
	if(!err && !GetStopped())
    {
        std::deque<ProtocolMessage>::iterator it;
        it = m_window.begin();
        unsigned int todrop = 0;
        if(it != m_window.end() && m_window.front().status() == ProtocolMessage::CREATED)
        {
            it++;
            for(; it != m_window.end(); it++)
            {
                if(MessageIsExpired(*it))
                    todrop++;
            }   
        }
        else
        {
            while(m_window.size() > 0 && m_window.front().status() != ProtocolMessage::CREATED && MessageIsExpired(m_window.front()))
            {
                Logger.Trace<<__PRETTY_FUNCTION__<<" Flushing"<<std::endl;
                //First message in the window should be the only one
                //ever to have been written.
                m_sendkills = true;
                Logger.Debug<<"Message Expired: "<<m_window.front().DebugString();
                m_window.pop_front();
                m_dropped++;
            }
        }
        if(m_dropped > MAX_DROPPED_MSGS || todrop > MAX_DROPPED_MSGS)
        {
            Logger.Warn<<"Connection to "<<GetUUID()<<" has lost "<<m_dropped<<" messages. Attempting to reconnect."<<std::endl;
            Stop();
            return;
        }
        Logger.Trace<<__PRETTY_FUNCTION__<<" Flushed Expired"<<std::endl;
        if(m_window.size() > 0)
        {
            if(m_sendkills &&  m_sendkill > m_window.front().sequence_num())
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
                m_window.front().set_kill(m_sendkill);
            }
        }
        WriteWindow();
        /// We use static pointer cast to convert the IPROTOCOL pointer to this
        /// derived type
        m_timeout.expires_from_now(boost::posix_time::milliseconds(CTimings::Get("CSRC_RESEND_TIME")));
        m_timeout.async_wait(boost::bind(&CProtocolSR::Resend,
            boost::static_pointer_cast<CProtocolSR>(shared_from_this()),
            boost::asio::placeholders::error));
    }
    Logger.Trace<<__PRETTY_FUNCTION__<<" Resend Finished"<<std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::ReceiveACK
/// @description Marks a message as acknowledged by the receiver and moves to
///     transmit the next message.
/// @pre A message has been sent.
/// @post If the ACK corresponds to the head of window by a match of sequence
///       number and the message hash, the the message is popped and the
///       killable flag is set to false, since the head of the window has never
///       been sent.
///       If the there is still an message in the window to send, the
///       resend function is called.
/// @param msg The received ACK message
///////////////////////////////////////////////////////////////////////////////
void CProtocolSR::ReceiveACK(const ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int seq = msg.sequence_num();
    if(m_window.size() > 0)
    {
        // Assuming hash collisions are small, we will check the hash
        // of the front message. On hit, we can accept the acknowledge.
        unsigned int fseq = m_window.front().sequence_num();
        Logger.Debug<<"Received ACK "<<seq<<" expecting ACK "<<fseq<<std::endl;
        google::protobuf::uint64 expectedHash = m_window.front().hash();
        if(fseq == seq && expectedHash == msg.hash())
        {
            m_sendkill = fseq;
            m_window.pop_front();
            m_sendkills = false;
            m_dropped = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::Receive
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
/// @param msg The received message
/// @return True if the message is accepted, false otherwise.
///////////////////////////////////////////////////////////////////////////////
bool CProtocolSR::Receive(const ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;  
    if(msg.status() == ProtocolMessage::BAD_REQUEST)
    {
        //See if we are already trying to sync:
        if(m_window.front().status() != ProtocolMessage::CREATED)
        {
			// See if we are getting a bad request we've already synced for.
            if(msg.hash() != m_outsynchash)
            {
                Logger.Debug<<"Syncronizing Connection (BAD REQUEST)"<<std::endl;
                m_outsynchash = msg.hash();
                SendSYN();
            }
            else
            {
                Logger.Debug<<"Already synced for this time"<<std::endl;
            }
        }
        return false;
    }
    else if(msg.status() == ProtocolMessage::CREATED)
    {
		boost::posix_time::ptime sendtime = boost::posix_time::time_from_string(msg.expire_time());
        //Check to see if we've already seen this SYN:
        if(sendtime == m_insynctime)
        {
		    Logger.Debug<<"Duplicate Sync"<<std::endl;
            return false;
        }
        Logger.Debug<<"Got Sync"<<std::endl;
        m_inseq = (msg.sequence_num()+1)%SEQUENCE_MODULO;
        m_insynctime = sendtime;
        m_inresyncs++;
        m_insync = true;
        SendACK(msg);
        return false;
    }
    else if(m_insync == false)
    {
        Logger.Debug<<"Connection Needs Resync"<<std::endl;
        //If the connection hasn't been synchronized, we want to
        //tell them it is a bad request so they know they need to sync.
        ProtocolMessage outmsg;
        // Presumably, if we are here, the connection is registered
        outmsg.set_status(ProtocolMessage::BAD_REQUEST);
        outmsg.set_hash(msg.hash());
        outmsg.set_sequence_num(m_inresyncs%SEQUENCE_MODULO);
        ProtocolMessageWindow cmsg;
        *cmsg.add_messages() = outmsg;
        Write(cmsg);
        return false;
    }
    else if(msg.status() == ProtocolMessage::MESSAGE)
    {
        unsigned int kill = 0;
        bool usekill = false; //If true, we should accept any inseq
        // See if the message contains kill data. If it does, read it and mark
        // we should use it.
        if (msg.has_kill())
        {
            kill = msg.kill();
            usekill = true;
        }
        else
        {
            kill = msg.sequence_num();
            usekill = false;
        }
        // This protocol NEEDS hashes
        if(msg.has_hash() == false)
        {
            return false;
        }
        //Consider the window you expect to see
        // If the killed message is the one immediately preceeding this
        // message in terms of sequence number we should accept it
        Logger.Debug<<"Recv: "<<msg.sequence_num()<<" Expected "<<m_inseq<<" Using kill: "<<usekill<<" with "<<kill<<std::endl;
        if(msg.sequence_num() == m_inseq)
        {
            m_inseq = (m_inseq+1)%SEQUENCE_MODULO;
            return true;
        }
        else if(usekill == true && kill < m_inseq && msg.sequence_num() > m_inseq)
        {
            //m_inseq will be right for the next expected message.
            m_inseq = (msg.sequence_num()+1)%SEQUENCE_MODULO;
            return true;
        }
        else if(usekill == true)
        {
            Logger.Debug<<"KILL: "<<kill<<" INSEQ "<<m_inseq<<" SEQ: "
                          <<msg.sequence_num()<<std::endl;
        }
        // Justin case.
        return false;
    }
    throw std::runtime_error("Receive didn't do anything with a message.");
    return false; 
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::SendACK
/// @description Composes an ack and PrepareAndWrites it to the channel. ACKS are saved
///     to the protocol's state and are written again during resends to try and
///     maximize througput.
/// @param msg The message to ACK.
/// @pre A message has been accepted.
/// @post The m_currentack member is set to the ack and the message will
///     be resent during resend until it expires.
///////////////////////////////////////////////////////////////////////////////
void CProtocolSR::SendACK(const ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int seq = msg.sequence_num();
    ProtocolMessage outmsg;
    // Presumably, if we are here, the connection is registered
    outmsg.set_status(ProtocolMessage::ACCEPTED);
    outmsg.set_sequence_num(seq);
    Logger.Debug<<"Generating ACK. Source exp time "<<msg.expire_time()<<std::endl;
    outmsg.set_expire_time(msg.expire_time());
    outmsg.set_hash(msg.hash());
    m_ack_window.push_back(outmsg);
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::SendSYN
/// @description Composes an SYN and writes it to the channel.
/// @pre A message has been accepted.
/// @post A syn has been written to the channel
///////////////////////////////////////////////////////////////////////////////
void CProtocolSR::SendSYN()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    unsigned int seq = m_outseq;
    if(m_window.size() == 0)
    {
        m_outseq = (m_outseq+1)%SEQUENCE_MODULO;
    }
    else
    {
        //Don't bother if front of queue is already a SYN
        if(m_window.front().has_status() && m_window.front().status() == ProtocolMessage::CREATED)
        {
            return;
        }
        //Set it as the seq before the front of queue
        seq = m_window.front().sequence_num();
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
    ProtocolMessage outmsg;
    outmsg.set_status(ProtocolMessage::CREATED);
    outmsg.set_sequence_num(seq);
    SetExpirationTimeFromNow(outmsg, boost::posix_time::millisec(CTimings::Get("CSRC_DEFAULT_TIMEOUT")));
    m_window.push_front(outmsg);
    m_outsync = true;
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::OnReceive
/// @description When a message is received, write the window to the channel,
///     then flush the ack queue.
/// @pre None
/// @post There are no acks queued and the message has been written to the
///     channel.
///////////////////////////////////////////////////////////////////////////////
void CProtocolSR::OnReceive()
{
    WriteWindow();
    m_ack_window.clear();
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::WriteWindow
/// @description Creates a message bundle of outstanding messages to write to
///     the channel.
/// @pre None
/// @post Writes the window to the channel.
//////////////////////////////////////////////////////////////////////////////
void CProtocolSR::WriteWindow()
{
    ProtocolMessageWindow outmsg;
    BOOST_FOREACH(const ProtocolMessage& msg, m_ack_window)
    {
        *outmsg.add_messages() = msg;
    }
    BOOST_FOREACH(const ProtocolMessage& msg, m_window)
    {
        *outmsg.add_messages() = msg;
    }
    if(outmsg.messages().size() > 0)
    {
        Write(outmsg);
    } 
}

///////////////////////////////////////////////////////////////////////////////
/// CProtocolSR::Write
/// @description Writes the message to the connected peer. Also, timestamps the
/// message, if a timestamp does not already exist.
/// @pre None
/// @post Writes the message to the channel.
/// @param msg the message write.
///////////////////////////////////////////////////////////////////////////////
void CProtocolSR::Write(ProtocolMessageWindow& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;


    IProtocol::Write(msg);
}

    }
}
