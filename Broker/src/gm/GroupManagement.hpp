//////////////////////////////////////////////////////////
/// @file         GroupManagement.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>,
///               Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// @description  Group Management Module
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
/// Technology, Rolla MO  65409 (ff@mst.edu).
/////////////////////////////////////////////////////////

#ifndef GROUPMANAGEMENT_HPP_
#define GROUPMANAGEMENT_HPP_


#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#include <cmath>
#include <sstream>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/progress.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include "CMessage.hpp"
#include "Utility.hpp"
#include "GMPeerNode.hpp"
#include "IAgent.hpp"
#include "IHandler.hpp"
#include "uuid.hpp"
#include "CDispatcher.hpp"
#include "CConnectionManager.hpp"
#include "CConnection.hpp"
#include "types/remotehost.hpp"

#include "Stopwatch.hpp"

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm {
  // namespace gm{

// Global constants
enum {
	CHECK_TIMEOUT = 10,
	TIMEOUT_TIMEOUT = 10,
	GLOBAL_TIMEOUT = 5
};

///////////////////////////////////////////////////////////////////////////////
///	@class			CAgent
///	@description	Declaration of Garcia-Molina Invitation Leader Election
///		algorithm.
///////////////////////////////////////////////////////////////////////////////

class GMAgent
  : public IReadHandler, public GMPeerNode,
    public Templates::Singleton< GMAgent >, 
    public IAgent< boost::shared_ptr<GMPeerNode> >
{
  friend class Templates::Singleton< GMAgent >;
  public:
    GMAgent();
    GMAgent(std::string uuid_, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, freedm::broker::CConnectionManager &m_conManager);
    GMAgent(const GMAgent&);
    GMAgent& operator=(const GMAgent&);
    ~GMAgent();
    
    // Internal
    void Recovery();
    bool IsCoordinator() const { return (Coordinator() == GetUUID()); };

    // Handlers
    virtual void HandleRead(broker::CMessage msg );
    
    //Routines
    void Check( const boost::system::error_code& err );
    void Timeout( const boost::system::error_code& err );
    void Recovery( const boost::system::error_code& err );
    void Premerge( const boost::system::error_code &err );
    void Merge( const boost::system::error_code& err );
    void PushPeerList();
    
    // Messages
    freedm::broker::CMessage AreYouCoordinator();
    freedm::broker::CMessage Invitation();
    freedm::broker::CMessage Ready();
    freedm::broker::CMessage Response(std::string msg,std::string type);
    freedm::broker::CMessage Accept();
    freedm::broker::CMessage AreYouThere();
    freedm::broker::CMessage PeerList();
 
    //Utility
    unsigned int MurmurHash2(const void * key, int len);

    // This is the main loop of the algorithm
    int	Run();
    void Stop();

    //Get/Set   
    //boost::asio::io_service& GetIOService() { return m_localservice; };
   
    //Peer Set Manipulation
    PeerNodePtr AddPeer(std::string uuid);
    PeerNodePtr AddPeer(PeerNodePtr peer);
    PeerNodePtr GetPeer(std::string uuid);

  protected:
    void InviteGroupNodes( const boost::system::error_code& err, PeerSet p_tempSet );
    void Reorganize( const boost::system::error_code& err );
    void SystemState();

    std::string Coordinator() const { return m_GroupLeader; }
    
    PeerSet	m_UpNodes;
    PeerSet m_Coordinators;
    PeerSet	m_AllPeers;
    PeerSet m_AYCResponse;
    PeerSet m_AYTResponse;
    
    // Mutex for protecting the m_UpNodes above
  	boost::mutex pList_Mutex;
    
    unsigned int m_GroupID;
    std::string  m_GroupLeader;
    unsigned int m_GrpCounter;
    
    /* IO and Timers */
    boost::asio::io_service m_localservice;
    boost::interprocess::interprocess_mutex m_timerMutex;
    deadline_timer m_timer;

    // Testing Functionality:
    int m_groupselection;
    int m_groupsformed;
    int m_groupsjoined;
    int m_groupsbroken;
    int m_rounds;
    Stopwatch m_electiontimer;
    Stopwatch m_ingrouptimer;
};

  }
//}

#endif
