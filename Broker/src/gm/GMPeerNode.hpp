//////////////////////////////////////////////////////////
/// @file         PeerNode.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Gropu Management peer node.
///
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
/// Technology, Rolla, MO  65409 (ff@mst.edu).
/////////////////////////////////////////////////////////

#ifndef GMPEERNODE_HPP_
#define GMPEERNODE_HPP_

#include <set>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "uuid.hpp"
#include "CBroker.hpp"
#include "CConnection.hpp"
#include "IPeerNode.hpp"


namespace freedm {

/// A container for an individual group management peer
class GMPeerNode : public IPeerNode {
    public:
        /// Constructs a peer.
        GMPeerNode(std::string uuid, ConnManagerPtr connmgr,
            boost::asio::io_service& ios,
            freedm::broker::CDispatcher& dispatch) :
                IPeerNode(uuid,connmgr,ios,dispatch) {};
        /// States this peer can be in.
        enum { NORMAL,DOWN,RECOVERY,REORGANIZATION,ELECTION };
};

} // End freedm

#endif
