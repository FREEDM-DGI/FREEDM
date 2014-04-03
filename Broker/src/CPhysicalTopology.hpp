////////////////////////////////////////////////////////////////////////////////
/// @file         CBroker.hpp
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

/// Central monolith of the Broker Architecture.
class CPhysicalTopology : private boost::noncopyable
{
public:
    typedef std::pair< std::string, std::string > VertexPair;
    typedef std::set<std::string> VertexSet;
    typedef std::map<std::string, VertexSet > AdjacencyListMap;
    typedef std::map<VertexPair, std::string> FIDControlMap;
    typedef std::map<std::string, bool> FIDState;

    /// Get the singleton instance of this class
    static CPhysicalTopology& Instance();

    /// Clean up this module
    ~CPhysicalTopology();

    /// Find the reachable peers.
    VertexSet ReachablePeers(std::string source, FIDState fidstate);

private:
    /// Private constructor for the singleton instance
    CPhysicalTopology();

    /// Load the topology from a file
    void LoadTopology();

    AdjacencyListMap m_adjlist; /// Structure of physical layer
    FIDControlMap m_fidcontrol; /// Which edges FIDs control.
};

    } // namespace broker
} // namespace freedm

#endif // FREEDM_PHYSICAL_TOPOLOGY_HPP

