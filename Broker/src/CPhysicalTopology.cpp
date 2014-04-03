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
static CPhysicalTopology& CPhysicalTopology::Instance()
{
    static CPhysicalTopology topology;
    return topology;
}

///////////////////////////////////////////////////////////////////////////////
/// Cleans up on exit
///////////////////////////////////////////////////////////////////////////////
~CPhysicalTopology::CPhysicalToplogy();

///////////////////////////////////////////////////////////////////////////////
/// Private constructor for the singleton instance
/// @pre None
/// @post LoadTopology has been called.
///////////////////////////////////////////////////////////////////////////////
CPhysicalTopology::CPhysicalTopology()
{
    LoadTopology();
}

/// Find the reachable peers.
std::set<string> CPhysicalTopology::ReachablePeers(std::string source, CPhysicalTopology::FIDState fidstate)
{
    typedef std::pair<int, std::string> BFSExplorer;
    typedef std::priority_queue< BFSExplorer > BFSPQueue;

    BFSPQueue openset;
    std::set<std::string> closedset;
    std::string consider, controlfid;
    CPhysicalToplogy::VertexPair vx;
    int hops;
    BFSExplorer tmp;

    openset.insert(BFSExplorer(0,source));
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
            if(m_fidcontrol.count(vx) > 0)
            {
                // An FID controls this edge
                controlfid = m_fidcontrol[neighbor];
                if(fidstate.count(controlfid) == 0 || fidstate[controlfid] == false)
                {
                    // If we don't have the state of an FID, assume it is OPEN.
                    // If the fid is OPEN (false) then that edge is not available.
                    continue;
                }
            }
            else
            {
                // This edge is not controlled by an FID, assume it is open.
                openset.insert(BFSExploer(hops+1, neighbor));
            }
        }
    } 
    return closedset;
}

/// Load the topology from a file
void LoadTopology()
{
    const std::string EDGE_TOKEN = "edge";
    const std::string VERTEX_TOKEN = "sst";
    const std::string CONTROL_TOKEN = "fid";
    
    AdjacencyListMap altmp;
    FIDControlMap fctmp;
    std::map<std::string, std::string> strans; //Fake to real translation table.
    VertexSet seennames;

    ifstream topf("config/topology.cfg");
    
    std::string token;
    
    if(!topf.is_open())
    {
        // XXX: raise exception, couldn't open topology.
    }

    while(!topf.eof())
    {
        topf >> token;
        if(token == EDGE_TOKEN)
        {
            std::string v_symbol1;
            std::string v_symbol2;
            topf>>v_symbol1>>v_symbol2;

            if(altmp.count[v_symbol1])
                altmp[v_symbol1] = VertexSet();
            if(altmp.count[v_symbol2])
                altmp[v_symbol2] = VertexSet();
            
            //Bi directional!
            altmp[v_symbol1].insert(v_symbol2);
            altmp(v_symbol2].insert(v_symbol1);

            seennames.insert(v_symbol1);
            seennames.insert(v_symbol2);
        }
        else if(token == VERTEX_TOKEN)
        {
            std::string uuid;
            std::string vsymbol;
            topf>>vsymbol>>uuid;
            symboltrans[vsymbol] = uuid;
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

            fctmp[vx1].insert(fidname);
            fctmp(vx2].insert(fidname);
            
            seennames.insert(v_symbol1);
            seennames.insert(v_symbol2);
        }
        else
        {
            // XXX: raise exception, malformed input
        }
    }
    // Verify that all the virtual names have a real translation.
    bool all_valid = true;
    BOOST_FOREACH( std::string vname, seennames )
    {
        if(strans.count(vname) == 0)
        {
            all_valid = false;
            // XXX: Warn user about bad name.
        }
    }
    if(all_valid == false)
    {
        // XXX: raise exception missing real name.
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

    BOOST_FOREACH( const FIDControlMap::value_type& mp, fctmp )
    {
        std::string namea = strans[mp.first.first];
        std::string nameb = strans[mp.first.second];
        std::string fidname = mp.second;
        m_fidcontrol[VertexPair(namea,nameb)] = fidname;        
    }
    // Done, yay!    
}

    } // namespace broker
} // namespace freedm

