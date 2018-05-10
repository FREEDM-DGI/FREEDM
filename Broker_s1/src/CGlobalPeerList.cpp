////////////////////////////////////////////////////////////////////////////////
/// @file         CGlobalPeerList.cpp
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

#include "CGlobalPeerList.hpp"
#include "CPeerNode.hpp"
#include "FreedmExceptions.hpp"

#include <stdexcept>
#include <string>

namespace freedm {

namespace broker {

////////////////////////////////////////////////////////
/// CGlobalPeerList::GetPeer
/// @description Fetch a peer based on uuid, throws an exception if they aren't found
/// @param uuid The UUID of the peer you are attempting to get
/// @return A CPeerNode object for the specified peer.
/// @ErrorHandling Runtime exception if no peer matches that description.
////////////////////////////////////////////////////////
CPeerNode CGlobalPeerList::GetPeer(const std::string& uuid)
{
    PeerSetIterator pst = Find(uuid);
    if(pst == end())
    {
        throw EDgiNoSuchPeerError("Peer " + uuid + " was not found in the global table");
    }
    return pst->second;
}
////////////////////////////////////////////////////////
/// CGlobalPeerList::Count
/// @description Count the number of peers with a specified uuid 
/// @param uuid The UUID of a peer.
/// @return A count of all nodes with that uuid in the table. (should be 1 or 0)
////////////////////////////////////////////////////////
int CGlobalPeerList::Count(const std::string& uuid)
{
    return m_peerlist.count(uuid);
}
///////////////////////////////////////////////////////
/// CGlobalPeerList::Find
/// @description Return an iterator to the peer with the given uuid
/// @param uuid The UUID of the peer you are attempting to access
/// @return An iterator to the peer if found or end() if not found.
///////////////////////////////////////////////////////
CGlobalPeerList::PeerSetIterator CGlobalPeerList::Find(const std::string& uuid)
{
    return m_peerlist.find(uuid);
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::begin
/// @description Return an iterator to the beginning of the container's table.
/// @return An iterator to the beginning of the peer list.
//////////////////////////////////////////////////////
CGlobalPeerList::PeerSetIterator CGlobalPeerList::begin()
{
    return m_peerlist.begin();
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::end
/// @description Return an iterator to the end of the container's table.
/// @return An iterator to the end of the peer list.
//////////////////////////////////////////////////////
CGlobalPeerList::PeerSetIterator CGlobalPeerList::end()
{
    return m_peerlist.end();
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::Insert
/// @description Pushes a peer node into the set
///	@pre None
/// @post p has been added to the global peer list.
/// @param p A CPeerNode to put into the container.
//////////////////////////////////////////////////////
void CGlobalPeerList::Insert(CPeerNode p)
{
    m_peerlist.insert(std::make_pair(p.GetUUID(),p));
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::Create
/// @description Adds a peer to the global peerlist by it's uuid
/// @pre None
///	@post If the peer is not already in the peerlist, it is inserted in
///		the global peerlist.
/// @param uuid The UUID of a peer.
/// @return A CPeerNode for the given uuid.
//////////////////////////////////////////////////////
CPeerNode CGlobalPeerList::Create(std::string uuid)
{
    if(Count(uuid) > 0)
    {
        return GetPeer(uuid);
    }
    CPeerNode p = CPeerNode(uuid);
    Insert(p);
    return p;
}
/////////////////////////////////////////////////////
/// CGlobalPeerList::PeerList
/// @description Gets a copy of the global peer list.
/// @return A copy of the global peer list
/////////////////////////////////////////////////////
CGlobalPeerList::PeerSet& CGlobalPeerList::PeerList()
{
    return m_peerlist;
}


}

}
