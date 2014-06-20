////////////////////////////////////////////////////////////////////////////////
/// @file           LoadBalance.hpp
///
/// @author         Ravi Akella <rcaq5c@mst.edu>
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    A distributed load balance algorithm for power management.
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

#ifndef LOAD_BALANCE_HPP
#define LOAD_BALANCE_HPP

#include "CBroker.hpp"
#include "CDevice.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"
#include "IDGIModule.hpp"
#include "messages/ModuleMessage.pb.h"

#include <map>
#include <set>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace freedm {
namespace broker {
namespace lb {

class LBAgent
    : public IDGIModule
{
public:
    LBAgent();
    int Run();
private:
    enum State { SUPPLY, DEMAND, NORMAL };
    //     
    /// Generates the message announcing current node state
    ModuleMessage MessageStateChange(std::string state);
    /// Generates message supply nodes send to demand nodes
    ModuleMessage MessageDraftRequest();
    /// Generates message demand nodes send in response to DraftRequest
    ModuleMessage MessageDraftAge(float age);
    /// Generates the message that the supply node uses to select a demand node.
    ModuleMessage MessageDraftSelect(float amount);
    /// Generates the message that the demand node uses to confirm the migration
    ModuleMessage MessageDraftAccept(float amount);
    /// Generates the message sent by the demand node to refuse migration.
    ModuleMessage MessageTooLate(float amount);
    /// Generates the message used to request a state collection.
    ModuleMessage MessageStateCollection();
    //// Generates the message used to announce the collected normal.
    ModuleMessage MessageCollectedState(float state);
    
    /// Boilerplate for preparing a message.
    ModuleMessage PrepareForSending(const LoadBalancingMessage & m, std::string recipient = "lb");
    /// Sends a message to all peers in a peerset.
    void SendToPeerSet(const PeerSet & ps, const ModuleMessage & m);

    /// First handler for an incoming message.
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> m, CPeerNode peer);
    /// Handles a node announcing its state change.
    void HandleStateChange(const StateChangeMessage & m, CPeerNode peer);
    /// Handles the draft request originating from the supply node.
    void HandleDraftRequest(const DraftRequestMessage & m, CPeerNode peer);
    /// Handles the draft age message coming from the demand node.
    void HandleDraftAge(const DraftAgeMessage & m, CPeerNode peer);
    /// Handles the draft select message coming from the supply node.
    void HandleDraftSelect(const DraftSelectMessage & m, CPeerNode peer);
    /// Handles the draft accept message coming from the demand node.
    void HandleDraftAccept(const DraftAcceptMessage & m, CPeerNode peer);
    /// Handles the draft reject message coming from the demand node.
    void HandleTooLate(const TooLateMessage & m);
    /// Handles the peerlist coming from the group leader.
    void HandlePeerList(const gm::PeerListMessage & m, CPeerNode peer);
    /// Handles the collected state coming from state collection
    void HandleCollectedState(const sc::CollectedStateMessage & m);
    /// Handles the collected state coming from load balancing
    void HandleCollectedState(const CollectedStateMessage & m);
    
    /// Moves a peer to the specified peerset.
    void MoveToPeerSet(PeerSet & ps, CPeerNode peer);
    
    /// The code that the supply nodes use to start doing migrations
    void LoadManage(const boost::system::error_code & error);
    /// The code that runs the firtst round of the LB phase
    void FirstRound(const boost::system::error_code & error);
    /// The code that runs after the draft request replies have arrived.
    void DraftStandard(const boost::system::error_code & error);
    
    /// Schedules the LoadManage that runs next round.
    void ScheduleNextRound();
    /// Updates the state from the devices.
    void ReadDevices();
    /// Updates the node's state.
    void UpdateState();
    /// Displays the load table to show DGI state.
    void LoadTable();
    /// Sends Draft request to all the demand peers.
    void SendDraftRequest();
    /// Sends draftage to the specified peer.
    void SendDraftAge(CPeerNode peer);
    /// Sends a draft select to the specified peer.
    void SendDraftSelect(CPeerNode peer, float step);
    /// Sends draft accept to the specified peer.
    void SendDraftAccept(CPeerNode peer, float step);
    /// Sends too late to the specified peer.
    void SendTooLate(CPeerNode peer, float step);
    /// Sets PStar to the specified level
    void SetPStar(float pstar);
    /// Sends the request to perform state collection.
    void ScheduleStateCollection();
    /// Synchronizes the Fast-Style Loadbalance with the physical system.
    void Synchronize(float k);
    /// Check the invariant prior to starting a new migration.
    bool InvariantCheck();

    /// The amount of time it takes to do an LB round
    const boost::posix_time::time_duration ROUND_TIME;
    /// The time it takes to get a draftrequest response
    const boost::posix_time::time_duration REQUEST_TIMEOUT;

    /// Timer handle for the round timer
    CBroker::TimerHandle m_RoundTimer;
    /// Timer handle for the request timer
    CBroker::TimerHandle m_WaitTimer;

    /// All peers in group.
    PeerSet m_AllPeers;
    /// Peers in the supply state
    PeerSet m_InSupply;
    /// Peers in the demand state
    PeerSet m_InDemand;
    /// Peers in the normal state
    PeerSet m_InNormal;

    /// The current state of this peer.
    State m_State;

    /// The gateway of this node.
    float m_Gateway;
    /// The amount of generation created by attached devices
    float m_NetGeneration;
    /// The gateway that we predict will be met by the devices.
    float m_PredictedGateway;
    /// The amount to migrate.
    float m_MigrationStep;
    /// The powerflow used by the physical invariant.
    float m_PowerDifferential;

    /// If the system is synchronized with the physical system.
    bool m_Synchronized;

    /// The coordinator of  the group.
    std::string m_Leader;
    /// Pending migrations.
    std::map<std::string, float> m_DraftAge;
};

} // namespace lb
} // namespace broker
} // namespace freedm

#endif // LOAD_BALANCE_HPP

