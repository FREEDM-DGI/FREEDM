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

namespace freedm {

namespace broker {

////////////////////////////////////////////////////////
/// CGlobalPeerList::GetPeer
/// @description Fetch a peer based on uuid, throws an exception if they aren't found
/// @param uuid The UUID of the peer you are attempting to access
/// @return A shared pointer to the node.
/// @ErrorHandling Runtime exception if no peer matches that description.
////////////////////////////////////////////////////////
CGlobalPeerList::PeerNodePtr CGlobalPeerList::GetPeer(std::string uuid)
{
    PeerSetIterator pst = Find(uuid);
    if(pst == end())
    {
        std::stringstream ss;
        ss << "Peer "<<uuid<<" was not found in the global table."<<std::endl;
        throw std::runtime_error(ss.str());
    }
    return (*pst).second;
}
////////////////////////////////////////////////////////
/// CGlobalPeerList::Count
/// @description Count the number of peers with a specified uuid (should be 1 or 0)
/// @param uuid The UUID of the peer you are attempting to access
/// @return A count of all nodes with that uuid in the table.
////////////////////////////////////////////////////////
int CGlobalPeerList::Count(std::string uuid)
{
    return m_peerlist.count(uuid);
}
///////////////////////////////////////////////////////
/// CGlobalPeerList::Find
/// @description Return an iterator to the peer with the given uuid
/// @param uuid The UUID of the peer you are attempting to access
/// @return An iterator to the peer if found or end() if not found.
///////////////////////////////////////////////////////
CGlobalPeerList::PeerSetIterator CGlobalPeerList::Find(std::string uuid)
{
    return m_peerlist.find(uuid);
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::begin
/// @description Return an iterator to the beginning of the container's table.
/// @return An iterator
//////////////////////////////////////////////////////
CGlobalPeerList::PeerSetIterator CGlobalPeerList::begin()
{
    return m_peerlist.begin();
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::end
/// @description Return an iterator to the end of the container's table.
/// @return An iterator
//////////////////////////////////////////////////////
CGlobalPeerList::PeerSetIterator CGlobalPeerList::end()
{
    return m_peerlist.end();
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::Insert
/// @description Pushes a peer node into the set
/// @param p A peernode pointer to put into the container.
//////////////////////////////////////////////////////
void CGlobalPeerList::Insert(PeerNodePtr p)
{
    m_peerlist.insert(std::pair<std::string, PeerNodePtr>(p->GetUUID(),p));
}
//////////////////////////////////////////////////////
/// CGlobalPeerList::Create
/// @description Constructs a new peernode pointer, intserts it, and returns it
/// @param uuid The UUID this pointer addresses.
/// @param connmgr The connection manager used to communicate with this node.
//////////////////////////////////////////////////////
CGlobalPeerList::PeerNodePtr CGlobalPeerList::Create(std::string uuid, ConnManagerPtr connmgr)
{
    if(Count(uuid) > 0)
    {
        return GetPeer(uuid);
    }
    PeerNodePtr p(new IPeerNode(uuid,connmgr));
    Insert(p);
    return p;
}
/////////////////////////////////////////////////////
/// CGlobalPeerList::PeerList
/// @description Returns a copy of the peerlist for iterating an stuff
/// @return A copy of the internal peer list
/////////////////////////////////////////////////////
CGlobalPeerList::PeerSet& CGlobalPeerList::PeerList()
{
    return m_peerlist;
}


}

}
