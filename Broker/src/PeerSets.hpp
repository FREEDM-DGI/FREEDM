////////////////////////////////////////////////////////////////////////////////
/// @file         PeerSets.hpp
///
/// @project      FREEDM DGI
///
/// @description  Provides functions for manipulating peersets 
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

#ifndef MPEERSETS_HPP_
#define MPEERSETS_HPP_

#include <map>
#include <string>

#include <boost/date_time/posix_time/ptime.hpp>

#include "CPeerNode.hpp"

namespace freedm {
namespace broker {

/// Provides a PeerSet type for a module templated on T
typedef std::map<std::string, CPeerNode> PeerSet;
/// Provides a PeerSet iterator templated on T
typedef PeerSet::iterator PeerSetIterator;
/// Provides count() for a PeerSet
int CountInPeerSet(PeerSet& ps, const CPeerNode& m);
/// Provides find() for a PeerSet
PeerSetIterator FindInPeerSet(PeerSet& ps, const CPeerNode& m);
/// Provides erase() for a PeerSet
void EraseInPeerSet(PeerSet& ps, const CPeerNode& m);
/// Provides insert() for a PeerSet
void InsertInPeerSet(PeerSet& ps, const CPeerNode& m);

/// Similar to a PeerSet, but also tracks the time a peer was inserted
typedef std::map<std::string,
                 std::pair<CPeerNode, boost::posix_time::ptime> > TimedPeerSet;

/// Provides a TimedPeerSet iterator templated on T
typedef TimedPeerSet::iterator TimedPeerSetIterator;

/// Provides count() for a TimedPeerSet
int CountInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m);

/// Get the time a peer was placed into the TimedPeerSet; only sensible if the peer is in the set exactly once
boost::posix_time::ptime GetTimeFromPeerSet(TimedPeerSet& tps, const CPeerNode& m);

/// Provides erase() for a TimedPeerSet
void EraseInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m);

/// Provides insert() for a TimedPeerSet
void InsertInTimedPeerSet(TimedPeerSet& tps,
                                 const CPeerNode& m,
                                 boost::posix_time::ptime time);

} // namespace freedm
} // namespace broker

#endif // MPEERSET_HPP_
