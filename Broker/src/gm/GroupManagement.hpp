//////////////////////////////////////////////////////////
/// @file         GroupManagement.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
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

#include "Stopwatch.hpp"

using boost::asio::ip::tcp;

using namespace boost::asio;

namespace freedm {
  // namespace gm{

///	Declaration of Garcia-Molina Invitation Leader Election algorithm.
class GMAgent
  : public IReadHandler, public GMPeerNode,
    public Templates::Singleton< GMAgent >, 
    public IAgent< boost::shared_ptr<GMPeerNode> >
{
  friend class Templates::Singleton< GMAgent >;
  public:
    /// Default constructor
    GMAgent();
    /// Constructor for using this object as a module.
    GMAgent(std::string uuid_, boost::asio::io_service &ios, freedm::broker::CDispatcher &p_dispatch, freedm::broker::CConnectionManager &m_conManager);
    /// Copy constructor for the module
    GMAgent(const GMAgent&);
    /// Copy constructor for the module
    GMAgent& operator=(const GMAgent&);
    /// Module destructor
    ~GMAgent();
    
    // Internal
    /// Resets the algorithm to the default startup state.
    void Recovery();
    /// Returns true if this node considers itself a coordinator
    bool IsCoordinator() const { return (Coordinator() == GetUUID()); };

    // Handlers
    /// Handles receiving incoming messages.
    virtual void HandleRead(broker::CMessage msg );
    
    //Routines
    /// Checks for other up leaders
    void Check( const boost::system::error_code& err );
    /// Checks that the leader is still alive and working
    void Timeout( const boost::system::error_code& err );
    /// Handles no response from timeout message
    void Recovery( const boost::system::error_code& err );
    /// Waits a time period determined by UUID for merge
    void Premerge( const boost::system::error_code &err );
    /// Sends invitations to all known nodes.
    void Merge( const boost::system::error_code& err );
    /// Sends the peer list to all group members.
    void PushPeerList();
    
    // Messages
    /// Creates AYC Message.
    freedm::broker::CMessage AreYouCoordinator();
    /// Creates Group Invitation Message
    freedm::broker::CMessage Invitation();
    /// Creates Ready Message
    freedm::broker::CMessage Ready();
    /// Creates A Response message
    freedm::broker::CMessage Response(std::string payload,std::string type,
        const boost::posix_time::ptime& exp);
    /// Creates an Accept Message
    freedm::broker::CMessage Accept();
    /// Creates a AYT, used for Timeout
    freedm::broker::CMessage AreYouThere();
    /// Generates a peer list
    freedm::broker::CMessage PeerList();
 
    // This is the main loop of the algorithm
    /// Called to start the system
    int	Run();
    /// Called on program termination
    void Stop();

    //Get/Set   
    //boost::asio::io_service& GetIOService() { return m_localservice; };
   
    //Peer Set Manipulation
    /// Adds a peer to the peer set from UUID
    PeerNodePtr AddPeer(std::string uuid);
    /// Adds a peer from a pointer to a peer node object
    PeerNodePtr AddPeer(PeerNodePtr peer);
    /// Gets a pointer to a peer from UUID.
    PeerNodePtr GetPeer(std::string uuid);

  protected:
    /// Sends invitations to all group members
    void InviteGroupNodes( const boost::system::error_code& err, PeerSet p_tempSet );
    /// Puts the system into the working state
    void Reorganize( const boost::system::error_code& err );
    /// Outputs information about the current state to the logger.
    void SystemState();
    /// Start the monitor after transient is over
    void StartMonitor(); // had parameter const boost::system::error_code& err
    /// Returns the coordinators uuid.
    std::string Coordinator() const { return m_GroupLeader; }
    
    /// Nodes In My Group
    PeerSet	m_UpNodes;
    /// Known Coordinators
    PeerSet m_Coordinators;
    /// All known peers
    PeerSet	m_AllPeers;
    /// Nodes expecting AYC response from
    PeerSet m_AYCResponse;
    /// Nodes expecting AYT response from
    PeerSet m_AYTResponse;
    
    // Mutex for protecting the m_UpNodes above
    boost::mutex pList_Mutex;
    
    /// The ID number of the current group (Never initialized for fun)
    unsigned int m_GroupID;
    /// The uuid of the group leader
    std::string  m_GroupLeader;
    /// The number of groups being formed
    unsigned int m_GrpCounter;
    
    /* IO and Timers */
    /// The io_service used.
    boost::asio::io_service m_localservice;
    /// A mutex to make the timers threadsafe
    boost::interprocess::interprocess_mutex m_timerMutex;
    /// A timer for stepping through the election process
    deadline_timer m_timer;
    /// What I like to call the TRANSIENT ELIMINATOR
    deadline_timer m_transient;
    
    // Testing Functionality:
    /// Counts the number of elections
    int m_groupselection;
    /// Counts the number of groups formed
    int m_groupsformed;
    /// Counts the number of groups this node as accepted invites to.
    int m_groupsjoined;
    /// Counts the number of groups this node has left
    int m_groupsbroken;
    /// The number of elections that have occured.
    int m_rounds;
    /// The running total of group membership.
    int m_membership;
    /// The number of times we've checked it
    int m_membershipchecks;
    Stopwatch m_electiontimer;
    Stopwatch m_ingrouptimer;

    // Timeouts
    boost::posix_time::time_duration CHECK_TIMEOUT;
    boost::posix_time::time_duration TIMEOUT_TIMEOUT;
    boost::posix_time::time_duration GLOBAL_TIMEOUT;
};

  }
//}

#endif
