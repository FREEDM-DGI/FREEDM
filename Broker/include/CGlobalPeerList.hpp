//////////////////////////////////////////////////////////
/// @file         CGlobalPeerList.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  A global store for all accepted peers
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
/// Technology, Rolla, MO  65409 (ff@mst.edu).
/////////////////////////////////////////////////////////

#ifndef CGLOBALPEERLIST_HPP
#define CGLOBALPEERLIST_HPP

#include <map>

#include <boost/shared_ptr.hpp>

#include "IPeerNode.hpp"

namespace freedm {

namespace broker {

// We have to forward declare GMAgent (and its namespace!) to make it a friend.
namespace gm {

class GMAgent;

}

class CGlobalPeerList : public boost::noncopyable
{
    public:
        friend class freedm::broker::IReadHandler;
        friend class freedm::broker::gm::GMAgent;
        /// Provides a PeerNodePtr type
        typedef boost::shared_ptr<IPeerNode> PeerNodePtr;
        /// The peerset type
        typedef std::map<std::string, PeerNodePtr> PeerSet;
        /// Provides and Iterator
        typedef PeerSet::iterator PeerSetIterator;
        /// Provides the global instance
        static CGlobalPeerList& instance()
        {
            static CGlobalPeerList inst;
            return inst;
        }
        /// Fetch a peer based on uuid, throws an exception if they aren't found
        PeerNodePtr GetPeer(std::string uuid);
        /// Count the number of peers with a specified uuid (should be 1 or 0)
        int Count(std::string uuid);
        /// Return an iterator to the peer with the given uuid
        PeerSetIterator Find(std::string uuid);
        /// Iterator to the beginning of the peerset
        PeerSetIterator begin();
        /// Iterator to the end of the peerset
        PeerSetIterator end();
    protected:
        /// Construct a peer from uuid and connection manager.
        PeerNodePtr Create(std::string uuid, ConnManagerPtr connmgr);
        /// Pushes a peer node into the set
        void Insert(PeerNodePtr p);
        /// Returns a copy of the peer map
        PeerSet& PeerList();
    private:
        /// The set of peers to present
        PeerSet m_peerlist;

};

}

}

#endif
