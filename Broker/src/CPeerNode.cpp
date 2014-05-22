////////////////////////////////////////////////////////////////////////////////
/// @file         CPeerNode.cpp
///
/// @author       Derek Ditch <dpdm85@mst.edu>
/// @author       Ravi Akella <rcaq5c@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Maintains communication with peer nodes
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
#include "CConnectionManager.hpp"
#include "CLogger.hpp"
#include "CPeerNode.hpp"

namespace freedm {

namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

/////////////////////////////////////////////////////////////
/// @fn CPeerNode::IPeerNode
/// @description Prepares a peer node. Provides node status
///   and sending functions to the agent in a very clean manner.
/// @param uuid The uuid of the node
/////////////////////////////////////////////////////////////
CPeerNode::CPeerNode(std::string uuid)
    : m_uuid(uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}


////////////////////////////////////////////////////////////
/// @fn CPeerNode::GetUUID
/// @description Returns the uuid of this peer node as a
///              string.
/////////////////////////////////////////////////////////////
std::string CPeerNode::GetUUID() const
{
    return m_uuid;
}

/////////////////////////////////////////////////////////////
/// @fn CPeerNode::GetHostname
/// @description Returns the hostname of this peer node as a
///              string
/////////////////////////////////////////////////////////////
std::string CPeerNode::GetHostname() const
{
    return CConnectionManager::Instance().GetHostByUUID(GetUUID()).hostname;
}
////////////////////////////////////////////////////////////
/// CPeerNode::GetPort
/// @description Returns the port number this node communicates on
////////////////////////////////////////////////////////////
std::string CPeerNode::GetPort() const
{
    return CConnectionManager::Instance().GetHostByUUID(GetUUID()).port;
}

/////////////////////////////////////////////////////////////
/// @fn CPeerNode::GetConnection
/// @description Uses the connection manager to attempt to
///   get a connection pointer to this node.
/// @pre None
/// @post If enough is known about the uuid, a connection
///   will exist with the connection manager.
/// @return A ConnectionPtr for the connection to this peer.
/////////////////////////////////////////////////////////////
broker::ConnectionPtr CPeerNode::GetConnection()
{
    return CConnectionManager::Instance().GetConnectionByUUID(m_uuid);
}

/////////////////////////////////////////////////////////////
/// @fn CPeerNode::Send
/// @description This method will attempt to construct a
///   connection to the peer this object represents and send
///   a message. Before, when this was done with TCP, it was
///   synchronous and would return when the message was sent
///   now, we use UDP and it doesn't matter.
/// @pre None
/// @post A message is sent to the peer represented by this
///   object
/// @param msg The message to write to channel
/// @return True if the message was sent.
/////////////////////////////////////////////////////////////
bool CPeerNode::Send(const freedm::broker::CMessage msg)
{
    try
    {
        broker::ConnectionPtr c = GetConnection();
        if(c.get() != NULL)
        {
            //Schedule the send with the io_service thread
            c->Send(msg);
        }
        else
        {
            Logger.Warn << "Couldn't Send Message To Peer (Couldn't make connection)" << std::endl;
            return false;
        }
    }
    catch(boost::system::system_error& e)
    {
        Logger.Warn << "Couldn't Send Message To Peer (Sending Failed)" << std::endl;
        return false;
    }
    Logger.Debug << "Sent message to peer" << std::endl;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
/// @fn operator==
/// @description Compares two peernodes.
/// @return True if the peer nodes have the same uuid.
///////////////////////////////////////////////////////////////////////////////
bool operator==(const CPeerNode& a, const CPeerNode& b)
{
  return (a.GetUUID() == b.GetUUID());
}
//////////////////////////////////////////////////////////////////////////////
/// @fn operator<
/// @description Provides a < operator for the maps these get stored in.
/// @return True if a's uuid is < b's.
/////////////////////////////////////////////////////////////////////////////
bool operator<(const CPeerNode& a, const CPeerNode& b)
{
  return (a.GetUUID() < b.GetUUID());
}

} // namespace broker

} // namespace freedm
