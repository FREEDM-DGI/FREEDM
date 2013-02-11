////////////////////////////////////////////////////////////////////////////////
/// @file         IAgent.hpp
///
/// @project      FREEDM DGI
///
/// @description  Base class for Broker modules.
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

#include <map>
#include <string>

// TODO - this shouldn't be included in a .hpp
//      - it's about time to create IAgent.cpp
//      - (providing a logger would also be good)
#include <algorithm>
#include <exception>

#ifndef IAGENT_HPP_
#define IAGENT_HPP_

namespace freedm {
namespace broker {

template<class T>
/// Common functions useful for all sorts of broker modules
class IAgent
{
public:
    /// Provides a PeerSet type for a module templated on T
    typedef std::map<std::string, T> PeerSet;
    /// Type of an element of a PeerSet
    /// Provides a PeerNodePtr templated on T
    typedef T PeerNodePtr;
    /// Provides a PeerSet iterator templated on T
    typedef typename PeerSet::iterator PeerSetIterator;
    /// Provides count() for a PeerSet
    static int CountInPeerSet(PeerSet& ps, T m)
        { return ps.count(m->GetUUID()); };
    /// Provides find() for a PeerSet
    static PeerSetIterator FindInPeerSet(PeerSet& ps, T m)
        { return ps.find(m->GetUUID()); };
    /// Provides erase() for a PeerSet
    static void EraseInPeerSet(PeerSet& ps, T m)
        { ps.erase(m->GetUUID()); };
    /// Provides insert() for a PeerSet
    static void InsertInPeerSet(PeerSet& ps, T m)
        { ps.insert(std::pair<std::string,T>(m->GetUUID(),m)); };

    /// Similar to a PeerSet, but also tracks the time a peer was inserted
    typedef std::map<std::string,
                     std::pair<T, boost::posix_time::ptime> > TimedPeerSet;

    /// Provides a TimedPeerSet iterator templated on T
    typedef typename TimedPeerSet::iterator TimedPeerSetIterator;

    /// Provides count() for a TimedPeerSet
    static int CountInTimedPeerSet(TimedPeerSet& tps, T m)
        {
            ssize_t count = 0;
            for (TimedPeerSetIterator it = tps.begin(); it != tps.end(); it++)
            {
                if (it->first == m->GetUUID())
                {
                    count++;
                }
            }
            return count;
        }

    /// Get the time a peer was placed into the TimedPeerSet; only sensible if the peer is in the set exactly once
    static boost::posix_time::ptime GetTimeFromPeerSet(TimedPeerSet& tps, T m)
        {
            for (TimedPeerSetIterator it = tps.begin(); it != tps.end(); it++)
            {
                if (it->first == m->GetUUID())
                {
                    return it->second.second;
                }
            }
            throw std::runtime_error("Expected peer wasn't found in peer set");
        }

    /// Provides erase() for a TimedPeerSet
    static void EraseInTimedPeerSet(TimedPeerSet& tps, T m)
        {
            for (TimedPeerSetIterator it = tps.begin(); it != tps.end(); )
            {
                if (it->first == m->GetUUID())
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

    /// Provides insert() for a TimedPeerSet
    static void InsertInTimedPeerSet(TimedPeerSet& tps, 
                                     T m,
                                     boost::posix_time::ptime time)
        {
            tps[m->GetUUID()] = std::make_pair(m, time);
        }
};

} // namespace freedm
} // namespace broker

#endif // IAGENT_HPP_
