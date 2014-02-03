////////////////////////////////////////////////////////////////////////////////
/// @file         IPeerNode.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Ravi Akella <rcaq5c@mst.edu>
/// @author       Derek Ditch <dpdm85@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Header file for peernode.cpp
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

#ifndef IPEERNODE_HPP_
#define IPEERNODE_HPP_

#include "CConnection.hpp"

#include <list>
#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace freedm {

namespace broker {

typedef boost::shared_ptr<freedm::broker::CMessage> MessagePtr;

/// Base interface for agents/broker modules
class IPeerNode
  : private boost::noncopyable
{
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
    public:
        /// Construct a peer node
        IPeerNode(std::string uuid);
        /// Gets the uuid of the node this addresses
        std::string GetUUID() const;
        /// Gives a connection ptr to this peer
        ConnectionPtr GetConnection();
        /// Gets the hostname of this peer
        std::string GetHostname() const;
        /// Gets the port of this peer.
        std::string GetPort() const;
        /// Sends a message to peer
        bool Send(freedm::broker::CMessage msg);
    protected:
        friend class CAgent;
    private:
        std::string m_uuid; /// This node's uuid.
};

bool operator==(const IPeerNode& a, const IPeerNode& b);
bool operator<(const IPeerNode& a, const IPeerNode& b);

} // namespace broker
} // namespace freedm

#endif
