////////////////////////////////////////////////////////////////////////////////
/// @file         CPhysicalTopology.cpp
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

#include "CPhysicalTopology.hpp"
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"


#include <queue>
#include <fstream>

namespace freedm {
    namespace broker {


namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// Get the singleton instance of this class
///////////////////////////////////////////////////////////////////////////////
CPhysicalTopology& CPhysicalTopology::Instance()
{
    static CPhysicalTopology topology;
    return topology;
}

///////////////////////////////////////////////////////////////////////////////
/// Cleans up on exit
///////////////////////////////////////////////////////////////////////////////
CPhysicalTopology::~CPhysicalTopology()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Private constructor for the singleton instance
/// @pre None
/// @post LoadTopology has been called.
///////////////////////////////////////////////////////////////////////////////
CPhysicalTopology::CPhysicalTopology()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_available = false;
    LoadTopology();
}

///////////////////////////////////////////////////////////////////////////////
/// Indicates whether or not a physical topology is available
/// @pre None
/// @post None
/// @return True if a physical topology has been successfully loaded.
///////////////////////////////////////////////////////////////////////////////
bool CPhysicalTopology::IsAvailable()
{
    return m_available;
}

///////////////////////////////////////////////////////////////////////////////
/// Find the reachable peers. Performs a BFS on the physical topology starting
/// at a source vertex. Nodes with FID are checked for the status of their FID
/// If the FID status is unavailable or the FID is open, the edge is considered
/// Broken
/// @pre A physical topology has been loaded.
/// @post No change to class state.
/// @returns A set of UUIDs of hostnames that are still reachable.
/// @param source The initial vertex to perform BFS from.
/// @param fidstate a map that is FID Name -> State. A closed FID is true, an
///     open FID is false. If an FID is open edges it controls are not used.
///////////////////////////////////////////////////////////////////////////////
CPhysicalTopology::VertexSet CPhysicalTopology::ReachablePeers(std::string source,
    CPhysicalTopology::FIDState fidstate)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    typedef std::pair<int, std::string> BFSExplorer;
    typedef std::priority_queue< BFSExplorer > BFSPQueue;

    BFSPQueue openset;
    std::set<std::string> closedset;
    std::string consider, controlfid;
    CPhysicalTopology::VertexPair vx;
    int hops;
    BFSExplorer tmp;

    openset.push(BFSExplorer(0,source));
    while(openset.size() > 0)
    {
        tmp = openset.top();
        openset.pop();
        closedset.insert(consider);

        consider = tmp.second;
        hops = tmp.first;

        BOOST_FOREACH( std::string neighbor, m_adjlist[source] )
        {
            if(closedset.count(neighbor) > 0)
                continue;
            vx = CPhysicalTopology::VertexPair(consider,neighbor);
            bool good_edge = true;
            std::pair<FIDControlMap::iterator,FIDControlMap::iterator>
                range = m_fidcontrol.equal_range(vx);
            for(FIDControlMap::iterator it=range.first; it != range.second; it++)
            {
                controlfid = it->second;
                // An FID controls this edge
                if(fidstate.count(controlfid) == 0
                    || fidstate[controlfid] == false)
                {
                    // If we don't have the state of an FID, assume it is OPEN.
                    // If the fid is OPEN (false) then that edge is not
                    // available.
                    good_edge = false;
                    break;
                }
            }
            if(good_edge)
            {
                // This edge is not controlled by an FID, assume it is open.
                openset.push(BFSExplorer(hops+1, neighbor));
            }
        }
    } 
    return closedset;
}

///////////////////////////////////////////////////////////////////////////////
/// Load the topology from a file
/// @pre: The topology is correctly specified in a topology config file. The
///     CGlobalConfiguration class has an entry loaded with a working path to
///     the topology file.
/// @post: The topology is loaded into the related maps: the adjacency list is
///     loaded into m_adjlist, and the map of which edges FIDs control is
///     loaded into m_fidcontrol.
/// @returns None
///////////////////////////////////////////////////////////////////////////////
void CPhysicalTopology::LoadTopology()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    const std::string EDGE_TOKEN = "edge";
    const std::string VERTEX_TOKEN = "sst";
    const std::string CONTROL_TOKEN = "fid";
    
    CPhysicalTopology::AdjacencyListMap altmp;
    CPhysicalTopology::FIDControlMap fctmp;
    std::map<std::string, std::string> strans; //Fake to real translation table
    CPhysicalTopology::VertexSet seennames;

    std::string fp = CGlobalConfiguration::Instance().GetTopologyConfigPath();
    if(fp == "")
    {
        Logger.Warn<<"No topology configuration file specified"<<std::endl;
        return;
    }
    std::ifstream topf(fp.c_str());

    std::string token;
    
    if(!topf.is_open())
    {
        //raise exception, couldn't open topology.
        throw std::runtime_error("Physical Topology: Couldn't open topology file.");
    }

    // Read from the input file
    while(!topf.eof())
    {
        topf >> token;
        if(token == EDGE_TOKEN)
        {
            std::string v_symbol1;
            std::string v_symbol2;
            topf>>v_symbol1>>v_symbol2;

            if(altmp.count(v_symbol1))
                altmp[v_symbol1] = VertexSet();
            if(altmp.count(v_symbol2))
                altmp[v_symbol2] = VertexSet();
            
            //Bi directional!
            altmp[v_symbol1].insert(v_symbol2);
            altmp[v_symbol2].insert(v_symbol1);

            seennames.insert(v_symbol1);
            seennames.insert(v_symbol2);
        }
        else if(token == VERTEX_TOKEN)
        {
            std::string uuid;
            std::string vsymbol;
            topf>>vsymbol>>uuid;
            strans[vsymbol] = uuid;
        }
        else if(token == CONTROL_TOKEN)
        {
            std::string v_symbol1;
            std::string v_symbol2;
            std::string fidname;
            VertexPair vx1, vx2;

            topf>>v_symbol1>>v_symbol2>>fidname;
            // Bi directional!
            vx1 = VertexPair(v_symbol1, v_symbol2);
            vx2 = VertexPair(v_symbol2, v_symbol1);

            fctmp.insert(FIDControlMap::value_type(vx1,fidname));
            fctmp.insert(FIDControlMap::value_type(vx2,fidname));
            
            seennames.insert(v_symbol1);
            seennames.insert(v_symbol2);
        }
        else
        {
            Logger.Error<<"Expected control token, saw '"<<token<<"'"<<std::endl;
            // raise exception, malformed input
            throw std::runtime_error("Physical Topology: Input topology file is malformed.");
        }
    }
    topf.close();

    // Verify that all the virtual names have a real translation.
    bool all_valid = true;
    BOOST_FOREACH( std::string vname, seennames )
    {
        if(strans.count(vname) == 0)
        {
            all_valid = false;
            // Warn user about bad name.
            Logger.Error<<"Couldn't find UUID for virtualname "<<vname<<std::endl;
        }
    }
    if(all_valid == false)
    {
        // raise exception missing real name.
        throw std::runtime_error("Physical Topology: UUID for virtual name missing.");
    }

    // Now we have to take the temporary ones and translate that into the real ones (Ugh!)
    BOOST_FOREACH( const AdjacencyListMap::value_type& mp, altmp )
    {
        std::string name = strans[mp.first];
        VertexSet t = mp.second;
        VertexSet n;
        BOOST_FOREACH( std::string vname, t )
        {
            n.insert(strans[vname]);
        }
        m_adjlist[name] = n;
    }

    // Mark how edges are controlled.
    BOOST_FOREACH( const FIDControlMap::value_type& mp, fctmp )
    {
        std::string namea = strans[mp.first.first];
        std::string nameb = strans[mp.first.second];
        std::string fidname = mp.second;
        // Bidirectional
        m_fidcontrol.insert(FIDControlMap::value_type(VertexPair(namea,nameb), fidname));        
        m_fidcontrol.insert(FIDControlMap::value_type(VertexPair(nameb,namea), fidname));        
    }
    // Done, yay!
    m_available = true; // Mark that a topology loaded successfully.    
}

    } // namespace broker
} // namespace freedm

