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

#include "CBroker.hpp"
#include "CConnectionManager.hpp"
#include "CDispatcher.hpp"
#include "CLogger.hpp"
#include "CConnection.hpp"
#include "Messages.hpp"

#include "CProtocolSR.hpp"
// FIXME restore
//#include "CProtocolSU.hpp"
//#include "CProtocolSRSW.hpp"

#include <vector>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

namespace freedm {
namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// Create a connection to one particular peer
///
/// @param uuid the UUID of the peer to connect to
///////////////////////////////////////////////////////////////////////////////
CConnection::CConnection(std::string uuid)
  : m_socket(CBroker::Instance().GetIOService())
  , m_protocol(*this)
  , m_uuid(uuid)
  , m_reliability(100)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Stop
/// @description Stops the socket and cancels the timeout timer.
/// @pre Any initialized CConnection object.
/// @post The underlying socket is closed and the message timeout timer is
///        cancelled.
///////////////////////////////////////////////////////////////////////////////
void CConnection::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_protocol.Stop();
    GetSocket().close();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::ChangePhase
/// @description An event that gets called when the broker changes the current
///   phase.
/// @pre None
/// @post The sliding window protocol if it exists is stopped.
/// @param newround If true, the phase change is also the start of an entirely
///     new round.
///////////////////////////////////////////////////////////////////////////////
void CConnection::ChangePhase(bool newround)
{
    m_protocol.ChangePhase(newround);
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
/// @param msg The message to write to the channel, INVALIDATED by this call.
/// @param expire_in how long from now to set the expiration time, or
///   not_a_date_time to use the default
///////////////////////////////////////////////////////////////////////////////
void CConnection::Send(const DgiMessage& msg, const boost::posix_time::time_duration& expire_in)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // If the UUID of the recipient (The value stored by GetUUID of this
    // object) is the same as the this node's uuid (As stored by the
    // Connection manager) place the message directly into the received
    // Queue.
    if(m_uuid == CConnectionManager::Instance().GetUUID())
    {
        boost::shared_ptr<DgiMessage> copy = boost::make_shared<DgiMessage>();
        copy->CopyFrom(msg);
        CDispatcher::Instance().HandleRequest(copy, m_uuid);
    }
    else
    {
        m_protocol.Send(msg, expire_in);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::ReceiveACK
/// @description Handler for recieving acknowledgments from a sender.
/// @pre Initialized connection.
/// @post The message with sequence number has been acknowledged and all
///   messages sent before that message have been considered acknowledged as
///   well.
/// @param msg The message to consider as acknnowledged
///////////////////////////////////////////////////////////////////////////////
void CConnection::ReceiveACK(const google::protobuf::Message& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_protocol.ReceiveACK(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Receive
/// @description Handler for determineing if a received message should be ACKd
/// @pre Initialized connection.
/// @post The message with sequence number has been acknowledged and all
///   messages sent before that message have been considered acknowledged as
///   well.
/// @param msg The message to consider as acknnowledged
///////////////////////////////////////////////////////////////////////////////
bool CConnection::Receive(const google::protobuf::Message& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(m_protocol.Receive(msg))
    {
        m_protocol.SendACK(msg);
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the socket used by this node.
///
/// @return A reference to the socket used by this connection.
///////////////////////////////////////////////////////////////////////////////
boost::asio::ip::udp::socket& CConnection::GetSocket()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_socket;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the UUID of the DGI on the other end of this connection.
///
/// @return the peer's UUID
///////////////////////////////////////////////////////////////////////////////
std::string CConnection::GetUUID() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_uuid;
}

///////////////////////////////////////////////////////////////////////////////
/// Set the connection reliability for DCUSTOMNETWORK. 0 all packets are
/// artifically dropped. 100 means no packets are artifically dropped.
///
/// @param r between 0 and 100
///////////////////////////////////////////////////////////////////////////////
void CConnection::SetReliability(int r)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_reliability = r;
}

///////////////////////////////////////////////////////////////////////////////
/// Get the connection reliability for DCUSTOMNETWORK
///
/// @return percentage of packets that are allowed through
///////////////////////////////////////////////////////////////////////////////
int CConnection::GetReliability() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    return m_reliability;
}

    } // namespace broker
} // namespace freedm
