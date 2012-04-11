//////////////////////////////////////////////////////////
/// @file         CStateCollection.hpp
///
/// @author       Li Feng <lfqt5@mail.mst.edu>
///		  Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  state collection module
///
/// @functions List of functions and external entry points
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.

/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.

/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).

///
/////////////////////////////////////////////////////////
#ifndef CSTATECOLLECTION_HPP_
#define CSTATECOLLECTION_HPP_


#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include <cmath>
#include <sstream>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/progress.hpp>

#include "CMessage.hpp"
#include "Utility.hpp"
#include "SCPeerNode.hpp"
//#include "ExtensibleLineProtocol.hpp"
#include "IAgent.hpp"
#include "IHandler.hpp"
#include "CUuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "CConnection.hpp"

#include "device/CPhysicalDeviceManager.hpp"
#include "device/PhysicalDeviceTypes.hpp"

#include <map>

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm
{
// namespace sc{



///////////////////////////////////////////////////////////////////////////////
/// @class          SCAgent
/// @description    Declaration of Chandy-Lamport Algorithm
///                 Each node that wants to initiate the state collection records its
///                 local state and sends a marker message to all other peer nodes.
///       	    Upon receiving a marker for the first time, peer nodes record their local states
///                 and start recording any message from incoming channel until receive marker from
///                 other nodes (these messages belong to the channel between the nodes).
///////////////////////////////////////////////////////////////////////////////

class SCAgent : public IReadHandler, public SCPeerNode, public Templates::Singleton< SCAgent >,
    public IAgent< boost::shared_ptr<SCPeerNode> >
{
  friend class Templates::Singleton< SCAgent >;
  public:
    ///Constructor        
    SCAgent(std::string uuid, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, freedm::broker::CConnectionManager &m_connManager, freedm::broker::device::CPhysicalDeviceManager &m_phyManager);
    ///Copy constructor for the module
    SCAgent(const SCAgent&);
    SCAgent& operator=(const SCAgent&);
    ///Destructor
    ~SCAgent();
    //Handler
    ///Handle receiving messages      
    virtual void HandleRead(broker::CMessage msg);

  private:
    //Marker structure
    typedef std::pair< std::string, int >  StateVersion;
        
    //Internal
    ///Initiator starts state collection
    void    Initiate();
    ///Save local state
    void    TakeSnapshot();
    ///Peer sends collected states back to the initiator
    void    SendStateBack();
    ///Peer sends "Done" message to the initiator to indicate finishing sending states back
    void    SendDoneBack();
    ///Initiator sends collected states back to the request module
    void    StateResponse();
        
    // Messages
    ///Create a marker message
    freedm::broker::CMessage marker();          
        
    //Peer set operations
    ///Add a peer to peer set from UUID
    PeerNodePtr AddPeer(std::string uuid);
    ///Add a peer to peer set from a pointer to a peer node object
    PeerNodePtr AddPeer(PeerNodePtr peer);
    ///Get a pointer to a peer from UUID
    PeerNodePtr GetPeer(std::string uuid);

    ///collect states container and its iterator
    std::multimap<StateVersion, ptree> collectstate;
    std::multimap<StateVersion, ptree>::iterator it;
    
    ///count number of states    
    unsigned int m_countstate;
    ///count number of marker
    unsigned int m_countmarker;
    ///count number of "Done" messages
    unsigned int m_countdone;
        
    ///flag to indicate save channel message
    bool m_NotifyToSave;
        
    ///module that request state collection
    std::string m_module;
        
    ///current version of marker
    StateVersion        m_curversion;
    ///current state
    ptree               m_curstate;
        
    ///physical device manager
    freedm::broker::device::CPhysicalDeviceManager &m_phyDevManager;
    ///all known peers
    PeerSet m_AllPeers;

    //IO and Timers
    deadline_timer      m_TimeoutTimer;
        
};

}

#endif

