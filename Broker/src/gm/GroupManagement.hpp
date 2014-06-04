////////////////////////////////////////////////////////////////////////////////
/// @file         GroupManagement.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Group Management Module
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

#ifndef GROUPMANAGEMENT_HPP_
#define GROUPMANAGEMENT_HPP_

#include "CBroker.hpp"
#include "IDGIModule.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"

#include "messages/ModuleMessage.pb.h"

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace freedm {

namespace broker {

namespace gm {

/// Declaration of Garcia-Molina Invitation Leader Election algorithm.
class GMAgent
  : public IDGIModule
{
  public:
    /// Module states
    enum { NORMAL,DOWN,RECOVERY,REORGANIZATION,ELECTION };
    /// Constructor for using this object as a module.
    GMAgent();
    /// Module destructor
    ~GMAgent();
    /// Called to start the system
    int	Run();
    /// Handles Processing a PeerList
    static PeerSet ProcessPeerList(const PeerListMessage& msg);

  private:
    /// Resets the algorithm to the default startup state.
    void Recovery();
    /// Returns true if this node considers itself a coordinator
    bool IsCoordinator() const { return (Coordinator() == GetUUID()); };

    // Handlers
    /// Handles received messages
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    /// Hadles recieving peerlists
    void HandlePeerList(const PeerListMessage& msg,CPeerNode peer);
    /// Handles recieving accept messsages
    void HandleAccept(const AcceptMessage& msg,CPeerNode peer);
    /// Handles recieving are you coordinator messages
    void HandleAreYouCoordinator(const AreYouCoordinatorMessage& msg,CPeerNode peer);
    /// Handles recieving are you there messsages
    void HandleAreYouThere(const AreYouThereMessage& msg,CPeerNode peer);
    /// Handles recieving invite messages
    void HandleInvite(const InviteMessage& msg,CPeerNode peer);
    /// Handles recieving AYC responses
    void HandleResponseAYC(const AreYouCoordinatorResponseMessage& msg,CPeerNode peer);
    /// Handles recieving AYT responses
    void HandleResponseAYT(const AreYouThereResponseMessage& msg,CPeerNode peere);
    /// Handles recieving peerlist requests
    void HandlePeerListQuery(const PeerListQueryMessage& msg, CPeerNode peer);

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
    ModuleMessage AreYouCoordinator();
    /// Creates Group Invitation Message
    ModuleMessage Invitation();
    /// Creates A Response message
    ModuleMessage AreYouCoordinatorResponse(std::string payload, int seq);
    /// Creates A Response message
    ModuleMessage AreYouThereResponse(std::string payload,int seq);
    /// Creates an Accept Message
    ModuleMessage Accept();
    /// Creates a AYT, used for Timeout
    ModuleMessage AreYouThere();
    /// Generates a peer list
    ModuleMessage PeerList(std::string requester="all");
    /// Generates a CMessage that can be used to query for the group
    static ModuleMessage PeerListQuery(std::string requester);

    //Peer Set Manipulation
    /// Adds a peer to the peer set from UUID
    CPeerNode AddPeer(std::string uuid);
    /// Adds a peer from a pointer to a peer node object
    CPeerNode AddPeer(CPeerNode peer);
    /// Gets a pointer to a peer from UUID.
    CPeerNode GetPeer(const std::string& uuid);

    /// Gets the status of a node
    int GetStatus() const;
    /// Sets the status of the node
    void SetStatus(int status);
    /// Sends invitations to all group members
    void InviteGroupNodes( const boost::system::error_code& err, PeerSet p_tempSet );
    /// Puts the system into the working state
    void Reorganize( const boost::system::error_code& err );
    /// Outputs information about the current state to the logger.
    void SystemState();
    /// Start the monitor after transient is over
    void StartMonitor(const boost::system::error_code& err);
    /// Returns the coordinators uuid.
    std::string Coordinator() const { return m_GroupLeader; }


    /// Wraps a GroupManagementMessage in a ModuleMessage
    static ModuleMessage PrepareForSending(
        const GroupManagementMessage& message, std::string recipient = "gm");

    /// Nodes In My Group
    PeerSet m_UpNodes;
    /// Known Coordinators
    PeerSet m_Coordinators;
    /// Nodes expecting AYC response from
    TimedPeerSet m_AYCResponse;
    /// Nodes expecting AYT response from
    TimedPeerSet m_AYTResponse;

    /// The ID number of the current group (Never initialized for fun)
    google::protobuf::uint32 m_GroupID;
    /// The uuid of the group leader
    std::string  m_GroupLeader;
    /// The number of groups being formed
    unsigned int m_GrpCounter;

    /* IO and Timers */
    /// The io_service used.
    boost::asio::io_service m_localservice;
    /// A timer for stepping through the election process
    CBroker::TimerHandle m_timer;
    /// Timer for checking FIDs.
    CBroker::TimerHandle m_fidtimer;

    // Timeouts
    /// How long between AYC checks
    boost::posix_time::time_duration CHECK_TIMEOUT;
    /// How long beteween AYT checks
    boost::posix_time::time_duration TIMEOUT_TIMEOUT;
    /// How long to wait before checking attached FIDs
    boost::posix_time::time_duration FID_TIMEOUT;
    /// How long to wait for responses from other nodes.
    boost::posix_time::time_duration AYC_RESPONSE_TIMEOUT;
    /// How long to wait for responses from other nodes.
    boost::posix_time::time_duration AYT_RESPONSE_TIMEOUT;
    /// How long to wait for responses from other nodes.
    boost::posix_time::time_duration INVITE_RESPONSE_TIMEOUT;

    /// Number of groups formed
    int m_groupsformed ;
    /// Number of groups broken
    int m_groupsbroken ;
    /// Number of elections started
    int m_groupselection;
    /// Number of accepts sent
    int m_groupsjoined;
    /// Total size of groups after all checks.
    int m_membership;
    /// Number of membership checks
    int m_membershipchecks;
    /// A store for the status of this node
    int m_status;
    /// A store for the state of attached FIDs.
    std::map< std::string , bool > m_fidstate;
};

} // namespace gm

} // namespace broker

} // namespace freedm

#endif
