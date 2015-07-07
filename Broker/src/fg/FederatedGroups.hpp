#ifndef FEDERATEDGROUPS_HPP_
#define FEDERATEDGROUPS_HPP_

#include "IDGIModule.hpp"
#include "CPeerNode.hpp"
#include "PeerSets.hpp"
#include "CBroker.hpp"

#include "device/CFakeAdapter.hpp"

#include "messages/ModuleMessage.pb.h"

namespace freedm {
namespace broker {
namespace fg {

class FGAgent
    : public IDGIModule
{
    public:
    /// Constructs the FGAgent, sets the intial state.
    FGAgent();
    /// Destroys the FGAgent
    ~FGAgent();
    /// Runs the federated behavior
    int Run();
    /// Message that a demand coordinator sends to get power
    ModuleMessage Take();
    /// Message the supply coordinator sends to allow power to transfer
    ModuleMessage TakeResponse(bool response);
    /// Message for other algorithms to indicate demand
    static ModuleMessage Demand();

    /// Determines the state of the group and distributes it
    void Round(const boost::system::error_code & error);
    /// Handles a federated groups state message
    void HandleStateMessage(const StateMessage &m, const CPeerNode& peer);
    /// Handles an AYC response message from Group Management
    void HandleResponseAYCMessage(const gm::AreYouCoordinatorResponseMessage & m, const CPeerNode& peer);
    /// Handles a peer list message from Group Management
    void HandlePeerListMessage(const gm::PeerListMessage & m, const CPeerNode& peer);
    /// Handles a state change message from Load Balancing
    void HandleStateChangeMessage(const lb::StateChangeMessage &m, const CPeerNode& peer);
    /// Handles a take message from a demand group
    void HandleTakeMessage(const TakeMessage &m, const CPeerNode & peer);
    /// Handles a take response message from a potential supply group
    void HandleTakeResponseMessage(const TakeResponseMessage &m, const CPeerNode& peer);
    /// Handles a demand message from a power balancing algorithm
    void HandleDemandMessage(const DemandMessage &, const CPeerNode &);
    /// Prepares a federated groups message for sending
    static ModuleMessage PrepareForSending(const FederatedGroupsMessage& message, std::string recipient);
    
    /// Gets the ammount of incoming power from other groups
    static float GetIncoming();
    /// Gets the amount of power being sent to other groups.
    static float GetOutgoing();
    /// Sets the amount of incoming power coming in from other groups.
    static void SetIncoming(float v);
    /// Sets the amount oof power going out to other groups
    static void SetOutgoing(float v);
    /// Changes the outgoing power by the specified amount
    static void ChangeOutgoing(float a);
    /// Changes the incoming power by the specified amount
    static void ChangeIncoming(float a);
    /// Returns true if Federated Groups considers this group to be in demand
    static bool IsDemand();
    
    private:
    /// Handles the incoming messages from other modules
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    /// Changes the Demand device value
    void SetIsDemand(bool d);
    /// Checks to make sure the virtual device is sane
    static void InvariantCheck();
    /// If true, this process is a coordinator and should do federated things.
    bool m_coordinator;
    /// The set of coordinators. AYC messages add / remove from this set.
    PeerSet m_coordinators;
    /// The set of groups which can supply power to other groups.
    PeerSet m_suppliers;
    /// The group state in terms of power
    int m_demandscore;
    /// The state of the virtual device. If true it is a sink (demand)
    bool m_vdev_sink;
    /// The collected AYC Response messages.
    StateMessage m_collection;
    /// Timer handle for the round timer
    CBroker::TimerHandle m_RoundTimer;
    /// The fake adapter for the virtual device.
    device::CFakeAdapter::Pointer m_vadapter;
    /// A store for global FIDs
    std::map< std::string, bool> m_fidstate;
};

} // namespace fg
} // namespace broker
} // namespace freedm

#endif
