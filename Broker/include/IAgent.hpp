#include <map>
#include <string>

#ifndef IAGENT_HPP_
#define IAGENT_HPP_

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
        int CountInPeerSet(PeerSet& ps, T m)
            { return ps.count(m->GetUUID()); };
        /// Provides find() for a PeerSet
        PeerSetIterator FindInPeerSet(PeerSet& ps, T m)
            { return ps.find(m->GetUUID()); };
        /// Provides erase() for a PeerSet
        void EraseInPeerSet(PeerSet& ps, T m)
            { ps.erase(m->GetUUID()); };
        /// Provides insert() for a PeerSet
        void InsertInPeerSet(PeerSet& ps, T m)
            { ps.insert(std::pair<std::string,T>(m->GetUUID(),m)); };
};

#endif
