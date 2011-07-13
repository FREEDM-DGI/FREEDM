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
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.

/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.

/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla,  MO  65409, (ff@mst.edu).
/////////////////////////////////////////////////////////

#include "CMessage.hpp"
#include <boost/thread/locks.hpp>
#include <map>

// Logging facility
#include "logger.hpp"

// Serialization and smart ptrs
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream>

#include "IPeerNode.hpp"

CREATE_EXTERN_STD_LOGS()

namespace freedm {

IPeerNode::IPeerNode(std::string uuid, ConnManagerPtr connmgr,
    boost::asio::io_service& ios, freedm::broker::CDispatcher& dispatch)
    : m_uuid(uuid),
      m_connmgr(connmgr),
      m_ios(ios),
      m_dispatch(dispatch)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
}

int IPeerNode::GetStatus() const
{
    return m_status;
}

void IPeerNode::SetStatus(int status)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_status = status;
}

ConnectionPtr IPeerNode::GetConnection()
{
    return m_connmgr.GetConnectionByUUID(m_uuid,m_ios,m_dispatch);
}

bool IPeerNode::Send(freedm::broker::CMessage msg)
{
    try
    {
        ConnectionPtr c = GetConnection();
        if(c.get() != NULL)
        {
            GetConnection()->Send(msg);
        }
        else
        {
            Logger::Info << "Couldn't Send Message To Peer (Couldn't make connection)" << std::endl;
            return false;
        }
    }
    catch(boost::system::system_error& e)
    {
        Logger::Info << "Couldn't Send Message To Peer (Sending Failed)" << std::endl;
        return false;
    }
    Logger::Debug << "Sent message to peer" << std::endl;
    return true;
}

void IPeerNode::AsyncSend(freedm::broker::CMessage msg)
{
    m_ios.post(boost::bind(&IPeerNode::Send, shared_from_this(), msg));
}

bool operator==(const IPeerNode& a, const IPeerNode& b)
{
  return (a.GetUUID() == b.GetUUID());
}
bool operator<(const IPeerNode& a, const IPeerNode& b)
{
  return (a.GetUUID() < b.GetUUID());
}

}
