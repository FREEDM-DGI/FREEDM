//////////////////////////////////////////////////////////
/// @file         IPeerNode.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///               Ravi Akella <rcaq5c@mst.edu>
///               Derek Ditch <dpdm85@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Header file for peernode.cpp
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
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
/////////////////////////////////////////////////////////

#ifndef IPEERNODE_HPP_
#define IPEERNODE_HPP_

#include <set>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#include "CUUIDHandler.hpp"
#include "uuid.hpp"
#include "CBroker.hpp"
#include "CConnection.hpp"
#include "CConnectionManager.hpp"


#include "logger.hpp"
//CREATE_EXTERN_STD_LOGS();


namespace freedm {
namespace broker {
  class CMessage;
  class CConnection;
}
typedef boost::shared_ptr<freedm::broker::CMessage> MessagePtr;
typedef freedm::broker::CConnectionManager& ConnManagerPtr;
typedef boost::shared_ptr<freedm::broker::CConnection> ConnectionPtr;
//////////////////////////////////////////////////////////
/// class IPeerNode
///
/// @description A container intended to be used generically to provide
///              basic IO and stracking of peer used in FREEDM agents.
///
/// @limitations This class is meant to be extended to be a specific
///              peer type for an agent, which is then further extended
///              to become an agent itself. See the generic agent interface.
///
/////////////////////////////////////////////////////////
class IPeerNode
  : public boost::enable_shared_from_this<IPeerNode>
  , private boost::noncopyable
{
    public:
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::IPeerNode
        /// @description Prepares a peer node. Takes in a connection
        ///   manager, io_service and dispatcher. Provides node status
        ///   and sending functions to the agent in a very clean manner.
        /////////////////////////////////////////////////////////////
        IPeerNode(std::string uuid, ConnManagerPtr connmgr,
            boost::asio::io_service& ios, freedm::broker::CDispatcher& dispatch);
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetStatus
        /// @description returns the status stored in the node as an
        ///   an integer. This means that in an inherited class, you
        ///   may either define integer constants or an enumeration
        ///   to generate status values.
        /////////////////////////////////////////////////////////////
        int GetStatus() const;
        ////////////////////////////////////////////////////////////
        /// @fn IPeerNode::SetStatus
        /// @description Sets the internal status variable, m_status
        ///   based on the paramter status. See GetStatus for tips
        ///   on establishing the status code numbers.
        ////////////////////////////////////////////////////////////
        void SetStatus(int status);
        ////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetUUID
        /// @description Returns the uuid of this peer node as a
        ///              string.
        /////////////////////////////////////////////////////////////
        std::string GetUUID() const { return m_uuid; };
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetHostname
        /// @description Returns the hostname of this peer node as a
        ///              string
        /////////////////////////////////////////////////////////////
        std::string GetHostname() const { return m_connmgr.GetHostnameByUUID(GetUUID()); };
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetConnection
        /// @description Uses the connection manager to attempt to
        ///   get a connection pointer to this node.
        /////////////////////////////////////////////////////////////
        ConnectionPtr GetConnection();
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetConnectionManager
        /// @description Returns a reference to the connection manager
        ///              this object was constructed with.
        /////////////////////////////////////////////////////////////
        ConnManagerPtr GetConnectionManager() { return m_connmgr; };
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetIOService
        /// @description Returns a reference to the IO_Service this
        ///   peer node was constructed with.
        /////////////////////////////////////////////////////////////
        boost::asio::io_service& GetIOService() { return m_ios; };
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::GetDispatcher
        /// @description Returns a reference to the dispatcher this
        ///   peer node was constructed with.
        /////////////////////////////////////////////////////////////
        freedm::broker::CDispatcher& GetDispatcher() { return m_dispatch; };
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::Send
        /// @description This method will attempt to construct a
        ///   connection to the peer this object represents and send
        ///   a message. This is done synchronously, and will not
        ///   return until the message has been sent. If you don't
        ///   need to send your message synchronously, then one should
        ///   try using AsyncSend, for better handling of downed nodes.
        /////////////////////////////////////////////////////////////
        bool Send(freedm::broker::CMessage msg);
        /////////////////////////////////////////////////////////////
        /// @fn IPeerNode::ASyncSend
        /// @description This function schedules the io_service to
        ///   call Send, which should allow many messages to be
        ///   eligible for sending. This may require multiple calls
        ///   to io_service run for maxium efficency.
        /////////////////////////////////////////////////////////////
        void AsyncSend(freedm::broker::CMessage msg);
    protected:
        friend class CAgent;
    private:
        std::string m_uuid; /// This node's uuid.
        ConnManagerPtr m_connmgr; /// The connection manager to use
        boost::asio::io_service& m_ios; /// io_service to connect with
        freedm::broker::CDispatcher& m_dispatch; /// Message handling dispatcher.
        int m_status;
};

bool operator==(const IPeerNode& a, const IPeerNode& b);
bool operator<(const IPeerNode& a, const IPeerNode& b);

} // End freedm

#endif
