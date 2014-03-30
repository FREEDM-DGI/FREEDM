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
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"
#include "CConnection.hpp"
#include "messages/DgiMessage.pb.h"

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
  , m_uuid(uuid)
  , m_reliability(100)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CConnection::Stop
/// @description Stops the socket
/// @pre Any initialized CConnection object.
/// @post The underlying socket is closed
///////////////////////////////////////////////////////////////////////////////
void CConnection::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
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
/// @param msg The message to write to the channel
///////////////////////////////////////////////////////////////////////////////
void CConnection::Send(const DgiMessage& msg)
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
        Write(msg);
    }
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

///////////////////////////////////////////////////////////////////////////////
/// Sends a message over the connection. This is a blocking send. An
/// asynchronous send is not currently provided because (a) this would require
/// a second io_service, or a second thread in our io_service's thread pool,
/// (b) because it would make the code more complicated: e.g. modules would be
/// required to use an async callback (write handler) if they want custom error
/// handling, and we would need to be more careful with our output buffer, and
/// (c) because we don't know whether this actually slows down the DGI or not,
/// or if so, how much. A TODO item would be to look into this. Synchronous
/// writes usually slow things down dramatically, but that may not be the case
/// with the DGI, given our custom scheduling.
///
/// @param msg the message to send to this p
///////////////////////////////////////////////////////////////////////////////
void CConnection::Write(const DgiMessage& msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // FIXME FIXME nasty, nasty; but makes the API easier to use
    const_cast<DgiMessage&>(msg).set_source_uuid(CGlobalConfiguration::Instance().GetUUID());
    const_cast<DgiMessage&>(msg).set_source_hostname(CGlobalConfiguration::Instance().GetHostname());
    const_cast<DgiMessage&>(msg).set_source_port(CGlobalConfiguration::Instance().GetListenPort());
    msg.CheckInitialized();

    // FIXME FIXME FIXME this is unreliable, but the DGI requires reliability. Switch to TCP.

    /// Check to make sure it isn't going to overfill our message packet
    if(msg.ByteSize() > CGlobalConfiguration::MAX_PACKET_SIZE)
    {
        Logger.Warn << "Message too long for buffer: " << std::endl
                << msg.DebugString() << std::endl;
        throw std::runtime_error("Outgoing message is too long for buffer");
    }

    #ifdef CUSTOMNETWORK
    if((rand()%100) >= GetConnection()->GetReliability())
    {
        Logger.Info<<"Outgoing Packet Dropped ("<<m_reliability<<") -> "<<m_uuid<<std::endl;
        return;
    }
    #endif

    boost::array<char, CGlobalConfiguration::MAX_PACKET_SIZE> write_buffer;
    msg.SerializeToArray(&write_buffer[0], CGlobalConfiguration::MAX_PACKET_SIZE);

    Logger.Debug<<"Writing "<<msg.ByteSize()<<" bytes to channel"<<std::endl;

    try
    {
        m_socket.send(boost::asio::buffer(&write_buffer[0], msg.ByteSize()));
    }
    catch(boost::system::system_error &e)
    {
        Logger.Debug << "Writing Failed: " << e.what() << std::endl;
        Stop(); // FIXME probably unimplemented
    }
}

    } // namespace broker
} // namespace freedm
