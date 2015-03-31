////////////////////////////////////////////////////////////////////////////////
/// @file         PeerSets.cpp
///
/// @project      FREEDM DGI
///
/// @description  Functions for manipulating peersets.
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&const CPeerNode& make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <exception>

#include "PeerSets.hpp"

namespace freedm {
namespace broker {

///////////////////////////////////////////////////////////////////////////////
/// CountInPeerSet
/// @description Counts the instances of a peer in a PeerSet. This function
///		provides count().
/// @param ps The peerset to search
/// @param m The peer to search for.
/// @pre None
/// @post None
/// @return The count of peers matching m (should be one or zero).
///////////////////////////////////////////////////////////////////////////////
int CountInPeerSet(PeerSet& ps, const CPeerNode& m)
{
    return ps.count(m.GetUUID());
}
///////////////////////////////////////////////////////////////////////////////
/// FindInPeerSet
/// @description Returns an iterator to the peer in the PeerSet. This function
///		provides find().
/// @param ps The peerset to search
/// @param m the peer to search for.
/// @pre None
/// @post None
/// @return an iterator to the specified Peer
///////////////////////////////////////////////////////////////////////////////
PeerSetIterator FindInPeerSet(PeerSet& ps, const CPeerNode& m)
{
    return ps.find(m.GetUUID()); 
}
///////////////////////////////////////////////////////////////////////////////
/// EraseInPeerSet
/// @description Removes the specified peer from a peer set.
///		Provides erase() for a PeerSet.
/// @param ps The peerset to erase from
/// @param m The peer to erase.
/// @pre None
/// @post If m is in the PeerSet, it is removed.
///////////////////////////////////////////////////////////////////////////////
void EraseInPeerSet(PeerSet& ps, const CPeerNode& m)
{
    ps.erase(m.GetUUID());
}
///////////////////////////////////////////////////////////////////////////////
/// InsertInPeerSet
/// @description Adds the specified peer to the PeerSet.
///		Provides insert() for a PeerSet.
/// @pre None
/// @post If m is not in the PeerSet it is added.
/// @param ps The peerset to search
/// @param m the peer to add to the set
///////////////////////////////////////////////////////////////////////////////
void InsertInPeerSet(PeerSet& ps, const CPeerNode& m)
{
    ps.insert(std::make_pair(m.GetUUID(),m));
}
///////////////////////////////////////////////////////////////////////////////
/// CountInPeerSet
/// @description Counts the instances of a peer in a TimedPeerSet. This function
///		provides count().
/// @param tps The peerset to search
/// @param m The peer to search for.
/// @pre None
/// @post None
/// @return The count of peers matching m (should be one or zero).
///////////////////////////////////////////////////////////////////////////////
int CountInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m)
{
    ssize_t count = 0;
    for (TimedPeerSetIterator it = tps.begin(); it != tps.end(); it++)
    {
        if (it->first == m.GetUUID())
        {
            count++;
        }
    }
    return count;
}
///////////////////////////////////////////////////////////////////////////////
/// GetTimeFromPeerSet
/// @description Get the time a peer was placed into the TimedPeerSet
/// @pre None
/// @post Throws an exception if the peer is not in the peerset.
/// @param tps The timed peer set to search.
/// @param m The peer to search for.
boost::posix_time::ptime GetTimeFromPeerSet(TimedPeerSet& tps, const CPeerNode& m)
{
    for (TimedPeerSetIterator it = tps.begin(); it != tps.end(); it++)
    {
        if (it->first == m.GetUUID())
        {
            return it->second.second;
        }
    }
    throw std::runtime_error("Expected peer wasn't found in peer set");
}

///////////////////////////////////////////////////////////////////////////////
/// EraseInTimedPeerSet
/// @description Removes the specified peer from a peer set.
///		Provides erase() for a TimedPeerSet.
/// @param tps The peerset to erase from
/// @param m The peer to erase.
/// @pre None
/// @post If m is in the TimedPeerSet, it is removed.
///////////////////////////////////////////////////////////////////////////////
void EraseInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m)
{
    for (TimedPeerSetIterator it = tps.begin(); it != tps.end(); )
    {
        if (it->first == m.GetUUID())
        {
            // careful not to invalidate the iterator
            tps.erase((it++)->first);
        }
        else
        {
            it++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// InsertInTimedPeerSet
/// @description Adds the specified peer to the TimedPeerSet.
///		Provides insert() for a TimedPeerSet.
/// @pre None
/// @post If m is not in the TimedPeerSet it is added.
/// @param tps The TimedPeerSet to insert into
/// @param m The peer to add to the set.
/// @param time The time the peer was added to the TimedPeerSet
///////////////////////////////////////////////////////////////////////////////
void InsertInTimedPeerSet(TimedPeerSet& tps,
                             const CPeerNode& m,
                             boost::posix_time::ptime time)
{
    tps[m.GetUUID()] = std::make_pair(m, time);
}

} // namespace freedm
} // namespace broker
