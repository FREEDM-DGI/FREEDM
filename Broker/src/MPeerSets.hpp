////////////////////////////////////////////////////////////////////////////////
/// @file         MPeerSets.hpp
///
/// @project      FREEDM DGI
///
/// @description  This is the peer-set mixin or CRTP. It provides functions
///               that most DGI modules will want to use.
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

#include <boost/chrono.hpp>

#include "CPeerNode.hpp"

namespace freedm {
namespace broker {

/// Common functions useful for all sorts of broker modules
class MPeerSets
{
public:
    /// Provides a PeerSet type for a module templated on T
    typedef std::map<std::string, CPeerNode> PeerSet;
    /// Provides a PeerSet iterator templated on T
    typedef typename PeerSet::iterator PeerSetIterator;
    /// Provides count() for a PeerSet
    static int CountInPeerSet(PeerSet& ps, const CPeerNode& m);
    /// Provides find() for a PeerSet
    static PeerSetIterator FindInPeerSet(PeerSet& ps, const CPeerNode& m);
    /// Provides erase() for a PeerSet
    static void EraseInPeerSet(PeerSet& ps, const CPeerNode& m);
    /// Provides insert() for a PeerSet
    static void InsertInPeerSet(PeerSet& ps, const CPeerNode& m);

    /// Similar to a PeerSet, but also tracks the time a peer was inserted
    typedef std::map<std::string,
                     std::pair<CPeerNode, boost::chrono::steady_clock::time_point> > TimedPeerSet;
    /// The type for the amount of time it takes to get a response
    typedef boost::chrono::duration<double> ChronoDuration;

    /// Provides a TimedPeerSet iterator templated on T
    typedef typename TimedPeerSet::iterator TimedPeerSetIterator;

    /// Provides count() for a TimedPeerSet
    static int CountInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m);

    /// Get the time since a peer was placed into the TimedPeerSet
    static boost::chrono::duration<double> GetTimeInPeerSet(TimedPeerSet& tps, const CPeerNode& m);

    /// Provides erase() for a TimedPeerSet
    static void EraseInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m);
    
    /// Provides insert() for a TimedPeerSet
    static void InsertInTimedPeerSet(TimedPeerSet& tps, const CPeerNode& m);
};

} // namespace freedm
} // namespace broker

#endif // MPEERSET_HPP_
