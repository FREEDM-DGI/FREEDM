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
    /// Provides a PeerNodePtr templated on T
    typedef T PeerNodePtr;
    /// Provides and Iterator templated on T
    typedef typename std::map<std::string, T>::iterator PeerSetIterator;
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
};

} // namespace freedm
} // namespace broker

#endif // IAGENT_HPP_
