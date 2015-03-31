////////////////////////////////////////////////////////////////////////////////
/// @file         CConnection.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a single outgoing connection to a client.
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

#include "CConnection.hpp"

#include "CBroker.hpp"
#include "CDispatcher.hpp"
#include "CLogger.hpp"
#include "CProtocolSR.hpp"
#include "messages/ModuleMessage.pb.h"
#include "messages/ProtocolMessage.pb.h"

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>

namespace freedm {
namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::CConnection
/// @description Create a connection to one particular peer. Initializes
///		the selected message protocol used to deliver messages.
/// @param uuid the UUID of the peer to connect to
/// @param endpoint The target to send the messages to for this connection.
///////////////////////////////////////////////////////////////////////////////
CConnection::CConnection(std::string uuid, boost::asio::ip::udp::endpoint endpoint)
  : m_protocol(boost::make_shared<CProtocolSR>(uuid,endpoint))   // FIXME hardcoded protocol
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Destructor
///////////////////////////////////////////////////////////////////////////////
CConnection::~CConnection()
{
    // Pass, protocol is smart pointer
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::Stop
/// @description Stops the protocol associated with this connection.
/// @pre None.
/// @post The message protocol's Stop method has been called.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_protocol->Stop();
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::GetStopped
/// @description Returns true if the underlying protocol has been stopped.
/// @return True if the underlying protocol is stopped.
///////////////////////////////////////////////////////////////////////////////
bool CConnection::GetStopped()
{
    return m_protocol->GetStopped();
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::ChangePhase
/// @description An event that gets called when the broker changes the current
///   phase.
/// @pre None
/// @post The protocol's ChangePhase event is called.
/// @param newround If true, the phase change is also the start of an entirely
///     new round.
///////////////////////////////////////////////////////////////////////////////
void CConnection::ChangePhase(bool newround)
{
    m_protocol->ChangePhase(newround);
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::Send
/// @description Passes a message to the protocol to deliver it to the intended
///		recipient. If the intended recipient is this process, the delivery
///		is done directly without the protocol.
/// @pre None.
/// @post The message is scheduled to be delivered.
/// @param msg The message to write to the channel.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Send(const ModuleMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // If the UUID of the recipient (The value stored by GetUUID of this
    // object) is the same as the this node's uuid, place the message directly
    // into the received Queue.
    if(m_protocol->GetUUID() == CGlobalConfiguration::Instance().GetUUID())
    {
        boost::shared_ptr<ModuleMessage> copy = boost::make_shared<ModuleMessage>();
        copy->CopyFrom(msg);
        CDispatcher::Instance().HandleRequest(copy, m_protocol->GetUUID());
    }
    else
    {
        m_protocol->Send(msg);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::ReceiveACK
/// @description Handler for recieving acknowledgments from the peer.
/// @pre Initialized connection.
/// @post Calls the protocol's ReceiveACK function
/// @param msg The received acknowledge message.
///////////////////////////////////////////////////////////////////////////////
void CConnection::ReceiveACK(const ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_protocol->ReceiveACK(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::Receive
/// @description Handler for receiving a non-ACK message from the peer
/// @pre Initialized connection.
/// @post Calls the protocol's Receive method.
/// @param msg The message received from the peer.
///////////////////////////////////////////////////////////////////////////////
bool CConnection::Receive(const ProtocolMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_protocol->Receive(msg))
    {
        m_protocol->SendACK(msg);
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::OnReceive
/// @description Handles performing some action after processing a received
///     window.
/// @pre None
/// @post Call's the protocol's OnReceive method.
///////////////////////////////////////////////////////////////////////////////
void CConnection::OnReceive()
{
    m_protocol->OnReceive();
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::GetUUID
/// @description Gets the UUID of the DGI on the other end of this connection.
/// @pre None
/// @post None
/// @return the peer's UUID
///////////////////////////////////////////////////////////////////////////////
std::string CConnection::GetUUID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_protocol->GetUUID();
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::SetReliability
/// @description Set the connection reliability for DCUSTOMNETWORK. 
/// @param r between 0 and 100 -- 0 meaning all packets are
/// artifically dropped. 100 means no packets are artifically dropped.
/// @pre The DGI has been compiled with DCUSTOMNETWORK
/// @post The reliability of the connection is changed
///////////////////////////////////////////////////////////////////////////////
void CConnection::SetReliability(int r)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_protocol->SetReliability(r);
}

///////////////////////////////////////////////////////////////////////////////
/// CConnection::GetReliability
/// @description Get the connection reliability for DCUSTOMNETWORK
/// @pre None
/// @post None
/// @return percentage of packets that are allowed through
///////////////////////////////////////////////////////////////////////////////
int CConnection::GetReliability() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_protocol->GetReliability();
}

    } // namespace broker
} // namespace freedm
