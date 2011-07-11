#include <map>
#include <string>

#ifndef IAGENT_HPP_
#define IAGENT_HPP_

template<class T>
class IAgent
{
    public:
        typedef std::map<std::string, T> PeerSet;
        typedef T PeerNodePtr;
        typedef typename std::map<std::string, T>::iterator PeerSetIterator;
        int CountInPeerSet(PeerSet& ps, T m)
            { return ps.count(m->GetUUID()); };
        PeerSetIterator FindInPeerSet(PeerSet& ps, T m)
            { return ps.find(m->GetUUID()); };
        void EraseInPeerSet(PeerSet& ps, T m)
            { ps.erase(m->GetUUID()); };
        void InsertInPeerSet(PeerSet& ps, T m)
            { ps.insert(std::pair<std::string,T>(m->GetUUID(),m)); };
};

#endif
