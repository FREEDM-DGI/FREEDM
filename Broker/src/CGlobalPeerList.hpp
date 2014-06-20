////////////////////////////////////////////////////////////////////////////////
/// @file         CGlobalPeerList.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  A global store for all accepted peers
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

#ifndef CGLOBALPEERLIST_HPP
#define CGLOBALPEERLIST_HPP

#include <map>
#include <string>

#include <boost/noncopyable.hpp>

namespace freedm {

namespace broker {

// We have to forward declare GMAgent (and its namespace!) to make it a friend.
namespace gm {

class GMAgent;

}

class CPeerNode;

class CGlobalPeerList
    : private boost::noncopyable
{
    public:
        friend class freedm::broker::gm::GMAgent;
        /// The peerset type
        typedef std::map<std::string, CPeerNode> PeerSet;
        /// Provides and Iterator
        typedef PeerSet::iterator PeerSetIterator;
        /// Provides the global instance
        static CGlobalPeerList& instance()
        {
            static CGlobalPeerList inst;
            return inst;
        }
        /// Fetch a peer based on uuid, throws an exception if they aren't found
        CPeerNode GetPeer(const std::string& uuid);
        /// Count the number of peers with a specified uuid (should be 1 or 0)
        int Count(const std::string& uuid);
        /// Return an iterator to the peer with the given uuid
        PeerSetIterator Find(const std::string& uuid);
        /// Iterator to the beginning of the peerset
        PeerSetIterator begin();
        /// Iterator to the end of the peerset
        PeerSetIterator end();
        /// Returns a copy of the peer map
        PeerSet& PeerList();
        /// Construct a peer
        CPeerNode Create(std::string uuid);
        /// Pushes a peer node into the set
        void Insert(CPeerNode p);
    private:
        /// The set of peers to present
        PeerSet m_peerlist;

};

}

}

#endif
