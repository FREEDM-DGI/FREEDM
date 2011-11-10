////////////////////////////////////////////////////////////////////
/// @file      CSRConnection.cpp
///
/// @author    Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Declare CConnection class
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
/// Computer Science, Missour University of Science and
/// Technology, Rolla, MO 65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include "CSRConnection.hpp"
#include "CMessage.hpp"
#include "CConnectionManager.hpp"
#include "IProtocol.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <set>

namespace freedm {
    namespace broker {

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
    //Sequence Numbers
    m_outseq = 0;
    m_inseq = 0;
    //Inbound Message Sequencing
    m_insync = false;
    //m_insynctime
    m_inresyncs = 0;
    //Outbound message sequencing
    m_outsync = false;
    m_outlastresync = SEQUENCE_MODULO;
    // Message killing
    m_sendkills = false;
    m_killable = false;
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::CSRConnection
/// @description Send function for the CSRConnection. Sending using this
///   protocol involves an alternating bit scheme. Messages can expire and 
///   delivery won't be attempted after the deadline is passed. Killed messages
///   are noted in the next outgoing message. The reciever tracks the killed
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
    ptree x = static_cast<ptree>(msg);
    unsigned int msgseq;

    if(m_outsync == false)
    {
        SendSYN();
    }

    CMessage outmsg(x);
    
    msgseq = m_outseq;
    outmsg.SetSequenceNumber(msgseq);
    m_outseq = (m_outseq+1) % SEQUENCE_MODULO;

    outmsg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
    outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHostname());
    outmsg.SetProtocol(GetIdentifier());
    outmsg.SetSendTimestampNow();

    m_window.push_back(outmsg);
    
    if(m_window.size() == 0)
    {
        m_killable = true;
        Write(outmsg);
        m_timeout.cancel();
        m_timeout.expires_from_now(boost::posix_time::milliseconds(REFIRE_TIME));
        m_timeout.async_wait(boost::bind(&CSRConnection::Resend,this,
            boost::asio::placeholders::error)); 
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::CSRConnection
/// @description Handles refiring ACKs and Sent Messages.
/// @pre The connection has recieved or sent at least one message.
/// @post One of the following conditions or combination of states is 
///       upheld:
///       1) An ack for a message that has not yet expired has been resent and
///          a timer to call resend has been set.
///       2) A message has expired for the first time and the message has been
///          written to the channel. This flips the sendkills
///          flag to one, indicating the protocol should begin appending the
///          kill field and the killed field to all outgoing messages.
///       3) A Message has expired, and it has been sent before, The killhash
///          member variable is set to the hash of that message. All messages
///          will note this message was killed until another message is marked
///          as killed.
///       4) The window is non-empty and a message is written to the channel,
///          the m_killable flag is set to true. The timer is set to resend
///       5) The window is empty and no message is set to the channel, the
///          timer is not re-set.
/// @param err The timer error code. If the err is 0 then the timer expired
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::Resend(const boost::system::error_code& err)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if(!err)
    {
        boost::posix_time::ptime now;
        now = boost::posix_time::microsec_clock::universal_time();
        bool firstcheck = true;
        CMessage ack;
        bool pushack = false;
        bool killed = false;
        // Check if the front of the queue is an ACK
        if(m_currentack.GetStatus() == freedm::broker::CMessage::Accepted)
        {
            if(!m_currentack.IsExpired())
            {
                Write(m_currentack);
                m_timeout.cancel();
                m_timeout.expires_from_now(boost::posix_time::milliseconds(REFIRE_TIME));
                m_timeout.async_wait(boost::bind(&CSRConnection::Resend,this,
                    boost::asio::placeholders::error));
            }
        }
        while(m_window.size() > 0 && m_window.front().IsExpired())
        {
            if(firstcheck == true && m_killable == true)
            {
                //First message in the window should be the only one
                //ever to have been written.
                m_sendkills = true;
                m_killhash = m_window.front().GetHash();
                killed = true;
            }
            firstcheck = false;
            m_window.pop_front();
        }
        if(m_window.size() > 0)
        {
            if(m_sendkills)
            {
                //Killhash should be set to the last message we tried to send
                //but wasn't accepted before the message expired. Since we
                //only ever have one outstanding message at time, we will keep
                //sending the hash of the last in channel message we killed.
                //On the reciever side, we will use some data structure to track
                //the last several killed messages. If we see them, we will drop
                //them. If a message is flagged with src.killed, the reciever
                //Should accept w/o considering the sequenceno
                ptree x;
                x.put("src.kill",m_killhash);
                x.put("src.killed",killed);
                m_window.front().SetProtocolProperties(x);
            }
            Write(m_window.front());
            // Head of window can be killed.
            m_killable = true;
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(REFIRE_TIME));
            m_timeout.async_wait(boost::bind(&CSRConnection::Resend,this,
                boost::asio::placeholders::error));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::RecieveACK
/// @description Marks a message as acknowledged by the reciever and moves to
///     transmit the next message.
/// @pre A message has been sent.
/// @post If the ACK corresponds to the head of window by a match of sequence
///       number and the message hash, the the message is popped and the
///       killable flag is set to false, since the head of the window has never
///       been sent.
///       If the there is still an message in the window to send, the
///       resend function is called.
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::RecieveACK(const CMessage &msg)
{
    unsigned int seq = msg.GetSequenceNumber();
    ptree pp = msg.GetProtocolProperties();
    size_t hash = pp.get<size_t>("src.hash");
    if(m_window.size() > 0)
    {
        // Assuming hash collisions are small, we will check the hash
        // of the front message. On hit, we can accept the acknowledge.
        unsigned int fseq = m_window.front().GetSequenceNumber();
        if(fseq == seq && m_window.front().GetHash() == hash)
        {
            m_window.pop_front();
            m_killable = false; //Head of window isn't killable; it hasn't been written.
        }
    }
    if(m_window.size() > 0)
    {
        boost::system::error_code x;
        Resend(x);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::Recieve
/// @description Accepts a message into the protocol, if that message should
///   be accepted.
/// @pre Accept logic can be complicated, there are several scenarios that
///      should be addressed.
///      1) The protocol has never recieved a message, and the first message
///         is recieved with or without a kill flag.
///      2) The protocol recieves a message without a killed flag (or the flag
///         has been set to false) with or without a kill hash.
///      3) The protocl recieves a message with a killed flag and a kill hash
///      4) A message arrives out of order, with a kill flag and hash
///      5) A message arrives out of order, without a kill flag.
///      6) A message arrives out of order, with a kill hash but no flag.
/// @post Cases are handled as followed:
///      1) The recieved message is accepted as the first message, and the
///         connection is marked as synchronized.
///      2) If the message is what we expect to see in terms of sequence numbers
///         it is accepted.
///      3) For 3, we assume the message arrives in the correct order. Therefore,
///         since it claims to be informing the protocol of a newly killed message
///         the hash it gives will not appear in the kill window. The hash is
///         appended to the kill window and the message is accepted.
///      4) For 4, an out of order message would appear after another message
///         has killed it, it will be dropped. However, if another message kills 
///         the message that kills it,  we can't conclude that the message is out of
///         order. Presumably then the message would arrive after its expiration
///         time (the message would have expired, and the message that killed it
///         would also have expired) If clocks are well set and running UTC,
///         it is highly unlikely this end (reciever) would not discard the
///         message as expired.
///      5) Scenario A) The message arrives out of order after expiring. A message
///         that kills it or kills the killer has come and been excepted. This means
///         that the protocol is expecting every subsequent message to include kill
///         messages. The out of order message would not be accepted on the basis
///         that it is missing casual information.
///         Scenario B) The message arives out of order before expiring. In this
///         case we rely on the sequence modulo to be sufficently large to prevent
///         a false accept.
///      6) A Message without a kill flag is subject to the same sequence checking
///         as five.
/// @return True if the message is accepted, false otherwise.
///////////////////////////////////////////////////////////////////////////////
bool CSRConnection::Recieve(const CMessage &msg)
{
    size_t kill = 0;
    bool killed = false; //If true, we should accept any inseq
    if(msg.GetStatus() == freedm::broker::CMessage::BadRequest)
    {
        //See if we are already trying to sync:
        if(m_window.front().GetStatus() != freedm::broker::CMessage::Created)
        {
            if(m_outlastresync != msg.GetSequenceNumber())
            {
                m_outlastresync = msg.GetSequenceNumber();
                SendSYN();
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
        }
        m_inseq = (msg.GetSequenceNumber()+1)%SEQUENCE_MODULO;
        m_insynctime = msg.GetSendTimestamp();
        m_inresyncs++;
        m_insync = true;
        SendACK(msg);
        return false;
    }
    if(m_insync == false)
    {
        //If the connection hasn't been synchronized, we want to
        //tell them it is a bad request so they know they need to sync.
        freedm::broker::CMessage outmsg;
        // Presumably, if we are here, the connection is registered 
        outmsg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
        outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHostname());
        outmsg.SetStatus(freedm::broker::CMessage::BadRequest);
        outmsg.SetSequenceNumber(m_inresyncs%SEQUENCE_MODULO);
        outmsg.SetProtocol(GetIdentifier());
        Write(outmsg);
        return false;
    }
    if(msg.IsExpired())
    {
        return false;
    }
    // See if this message matches any of our outstanding kills
    for(int i=0; i < m_killwindow.size(); i++)
    {
        if(msg.GetHash() == m_killwindow[i])
            return false;
    }
    //See if the message before this one was killed:
    //The killed flag being set indicates this message comes
    //directly after a kill. Any message sent directly after
    //A kill should be accepted
    try
    {
        ptree pp = msg.GetProtocolProperties();
        killed = pp.get<unsigned int>("src.killed");
    }
    catch(std::exception &e)
    {
        //pass
    }
    // See if there are any new kills.
    try
    {
        ptree pp = msg.GetProtocolProperties();
        kill = pp.get<size_t>("src.kill");
        //Since the kill is repeated, a message can't be arriving in order
        //if its kill is in the kill window (but not at the end) OR it
        //doesn't have a kill when the killwindow is not empty.
        for(int i=0; i < m_killwindow.size(); i++)
        {
            if(m_killwindow[i] != kill && i == m_killwindow.size()-1)
            {
                m_killwindow.push_back(kill);
                if(m_killwindow.size() > KILLWINDOW_SIZE)
                {
                    m_killwindow.pop_front();
                }
                break;
            }
            else if(m_killwindow[i] == kill && i == m_killwindow.size()-1)
            {
                // We have recieved a message. Its kill code is the last
                // message we have noted for killing. This means it has to
                // come after the message that we originally recieved the
                // kill code for. In this case, that message must have had
                // the kill flag set. If we see the kill flag set in this
                // circumstance then, we can assume its a repeat of a mess-
                // we have already recieved. And since a kill flag means an
                // abitrary accept, we will reject it under the assumption
                // We have seen it before. Otherwise, we can tentatively
                // accept the message.
                if(killed == true)
                    return false;
                else
                    break;
            }
            // The kill that we have been given is not the back of the list
            if(m_killwindow[i] == kill)
            {
                return false;
            }
        }
        
    }
    catch(std::exception &e)
    {
        if(m_killwindow.size() > 0)
        {
            // We use the kill code to help detect casualility;
            // We have been sent a message with no kill code, but
            // We expect to see one, since once a message has been
            // Killed, the messages will ALWAYS have a kill message
            // In them.
            return false;
        }
    }
    //Consider the window you expect to see
    if(msg.GetSequenceNumber() == m_inseq)
    {
        m_insync = true;
        m_inseq = (m_inseq+1)%2;
        return true;
    }
    else if(killed)
    {
        //m_inseq will be right for the next expected message.
        m_inseq = (msg.GetSequenceNumber()+1)%2;
        return true;
    }
    // Justin case.
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// CSRConnection::SendACK
/// @description Composes an ack and writes it to the channel. ACKS are saved
///     to the protocol's state and are written again during resends to try and
///     maximize througput.
/// @param The message to ACK.
/// @pre A message has been accepted.
/// @post The m_currentack member is set to the ack and the message will
///     be resent during resend until it expires.
///////////////////////////////////////////////////////////////////////////////
void CSRConnection::SendACK(const CMessage &msg)
{
    unsigned int seq = msg.GetSequenceNumber();
    freedm::broker::CMessage outmsg;
    ptree pp;
    pp.put("src.hash",msg.GetHash());
    // Presumably, if we are here, the connection is registered 
    outmsg.SetSourceUUID(GetConnection()->GetConnectionManager().GetUUID());
    outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHostname());
    outmsg.SetStatus(freedm::broker::CMessage::Accepted);
    outmsg.SetSequenceNumber(seq);
    outmsg.SetSendTimestampNow();
    outmsg.SetProtocol(GetIdentifier());
    outmsg.SetProtocolProperties(pp);
    outmsg.SetExpireTime(msg.GetExpireTime());
    Write(outmsg);
    m_currentack = outmsg;
    /// Hook into resend until the message expires.
    m_timeout.cancel();
    m_timeout.expires_from_now(boost::posix_time::milliseconds(REFIRE_TIME));
    m_timeout.async_wait(boost::bind(&CSRConnection::Resend,this,
        boost::asio::placeholders::error));
}

void CSRConnection::SendSYN()
{
    unsigned int seq = m_outseq;
    freedm::broker::CMessage outmsg;
    if(m_window.size() == 0)
    {
        m_outseq = (m_outseq+1)%SEQUENCE_MODULO;
    }
    else
    {
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
    outmsg.SetSourceHostname(GetConnection()->GetConnectionManager().GetHostname());
    outmsg.SetStatus(freedm::broker::CMessage::Created);
    outmsg.SetSequenceNumber(seq);
    outmsg.SetSendTimestampNow();
    outmsg.SetProtocol(GetIdentifier());
    Write(outmsg);
    m_window.push_front(outmsg);
    m_outsync = true;
    /// Hook into resend until the message expires.
    m_timeout.cancel();
    m_timeout.expires_from_now(boost::posix_time::milliseconds(REFIRE_TIME));
    m_timeout.async_wait(boost::bind(&CSRConnection::Resend,this,
        boost::asio::placeholders::error));
}

    }
}
