////////////////////////////////////////////////////////////////////////////////
/// @file         CPhysicalTopology.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implements a physical topology layer for the DGI
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

#ifndef FREEDM_PHYSICAL_TOPOLOGY_HPP
#define FREEDM_PHYSICAL_TOPOLOGY_HPP

#include <boost/noncopyable.hpp>
#include <set>
#include <map>
#include <string>

namespace freedm {
    namespace broker {

/// Provides the Physical Topology Architecture.
///////////////////////////////////////////////////////////////////////////////
/// Provides the physical topology of the power system. It gets read from a
/// file. There is documentation on the wiki. 
///
/// It's really important that we have this long documentation, even though
/// there is nothing to say that would actually be helpful: look at the only
/// (public) function in this class.
///////////////////////////////////////////////////////////////////////////////
class CPhysicalTopology : private boost::noncopyable
{
public:
    typedef std::pair< std::string, std::string > VertexPair;
    typedef std::set<std::string> VertexSet;
    typedef std::map<std::string, VertexSet > AdjacencyListMap;
    typedef std::multimap<VertexPair,  std::string> FIDControlMap;
    typedef std::map<std::string, bool> FIDState;

    /// Get the singleton instance of this class
    static CPhysicalTopology& Instance();
	
    /// Find the reachable peers.
    VertexSet ReachablePeers(std::string source, FIDState fidstate);

    /// Returns if the physical topology is available.
    bool IsAvailable();

private:
    /// Private constructor for the singleton instance
    CPhysicalTopology();

    /// Load the topology from a file
    void LoadTopology();

    /// Gets the realname from the virtual name
    std::string RealNameFromVirtual(std::string vname);

    AdjacencyListMap m_adjlist; /// Structure of physical layer
    FIDControlMap m_fidcontrol; /// Which edges FIDs control.
    bool m_available; /// If a physical topology has been loaded
    std::map<std::string, std::string> m_strans; /// Fake to real translation table
};

    } // namespace broker
} // namespace freedm

#endif // FREEDM_PHYSICAL_TOPOLOGY_HPP

