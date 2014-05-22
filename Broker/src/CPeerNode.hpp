////////////////////////////////////////////////////////////////////////////////
/// @file         CPeerNode.hpp
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

#ifndef CPEERNODE_HPP_
#define CPEERNODE_HPP_

#include <string>

namespace freedm {

namespace broker {

class ModuleMessage;

/// Base interface for agents/broker modules
class CPeerNode
{
    public:
        /// Construct a peer node
        CPeerNode();
        /// Construct a peer node
        CPeerNode(std::string uuid);
        /// Gets the uuid of the node this addresses
        std::string GetUUID() const;
        /// Gets the hostname of this peer
        std::string GetHostname() const;
        /// Gets the port of this peer.
        std::string GetPort() const;
        /// Sends a message to peer
        void Send(const ModuleMessage& msg);
    private:
        std::string m_uuid; /// This node's uuid.
};

bool operator==(const CPeerNode& a, const CPeerNode& b);
bool operator<(const CPeerNode& a, const CPeerNode& b);

} // namespace broker
} // namespace freedm

#endif
