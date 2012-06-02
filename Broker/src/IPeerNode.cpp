//////////////////////////////////////////////////////////
/// @file         PeerNode.cpp
///
/// @author       Derek Ditch <dpdm85@mst.edu>
///               Ravi Akella <rcaq5c@mst.edu>
///               Stephen Jackson <scj7t4@mst.edu>    
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Maintains communication with peer nodes
///
/// @functions List of functions and external entry points
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
/// Technology, Rolla,  MO  65409, (ff@mst.edu).
/////////////////////////////////////////////////////////

#include "CMessage.hpp"
#include <boost/thread/locks.hpp>
#include <map>

// Serialization and smart ptrs
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream>

#include "CConnection.hpp"
#include "IPeerNode.hpp"
#include "CLogger.hpp"

namespace freedm {

namespace broker {
        
namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

/////////////////////////////////////////////////////////////
/// @fn IPeerNode::IPeerNode
/// @description Prepares a peer node. Takes in a connection
///   manager, io_service and dispatcher. Provides node status
///   and sending functions to the agent in a very clean manner.
/// @param uuid The uuid of the node
/// @param connmgr The module managing the connections
/// @param ios The related ioservice used for scheduling
/// @param dispatch The dispatcher used to deliver messages
/////////////////////////////////////////////////////////////
IPeerNode::IPeerNode(std::string uuid, ConnManagerPtr connmgr)
    : m_uuid(uuid),
      m_connmgr(connmgr)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}


////////////////////////////////////////////////////////////
/// @fn IPeerNode::SetStatus
/// @description Sets the internal status variable, m_status
///   based on the paramter status. See GetStatus for tips
///   on establishing the status code numbers.
/// @param status The status code to set
/// @pre None
/// @post The modules status code has been set to "status"
////////////////////////////////////////////////////////////
void IPeerNode::SetStatus(int status)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_status = status;
}

/////////////////////////////////////////////////////////////
/// @fn IPeerNode::GetConnection
/// @description Uses the connection manager to attempt to
///   get a connection pointer to this node.
/// @pre None
/// @post If enough is known about the uuid, a connection 
///   will exist with the connection manager.
/// @return A ConnectionPtr for the connection to this peer.
/////////////////////////////////////////////////////////////
broker::ConnectionPtr IPeerNode::GetConnection()
{
    return m_connmgr.GetConnectionByUUID(m_uuid);
}

/////////////////////////////////////////////////////////////
/// @fn IPeerNode::Send
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
bool IPeerNode::Send(freedm::broker::CMessage msg)
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

/////////////////////////////////////////////////////////////
/// @fn IPeerNode::ASyncSend
/// @description Calls send. This function is depreciated by
///   our change to UDP. 
/////////////////////////////////////////////////////////////
void IPeerNode::AsyncSend(freedm::broker::CMessage msg)
{
    //Depcreciated by UDP.
    Send(msg);
}
///////////////////////////////////////////////////////////////////////////////
/// @fn operator==
/// @description Compares two peernodes.
/// @return True if the peer nodes have the same uuid.
///////////////////////////////////////////////////////////////////////////////
bool operator==(const IPeerNode& a, const IPeerNode& b)
{
  return (a.GetUUID() == b.GetUUID());
}
//////////////////////////////////////////////////////////////////////////////
/// @fn operator<
/// @description Provides a < operator for the maps these get stored in.
/// @return True if a's uuid is < b's.
/////////////////////////////////////////////////////////////////////////////
bool operator<(const IPeerNode& a, const IPeerNode& b)
{
  return (a.GetUUID() < b.GetUUID());
}

} // namespace broker

} // namespace freedm
